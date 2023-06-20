#include "Recorder.h"
#include <sstream>
#include <CCGL.h>
#include <filesystem>
#include <fstream>
#include "Utils.h"

Recorder::Recorder() : m_width(1280), m_height(720), m_fps(60) {}

void Recorder::Start(const std::string& path) {
#ifdef SHOW_CONSOLE
    static bool hasConsole = false;
    if(!hasConsole) {
        hasConsole = true;
        AllocConsole();
        static std::ofstream conout("CONOUT$", std::ios::out);
        std::cout.rdbuf(conout.rdbuf());
    }
#endif
    m_recording = true;
    m_frameHasData = false;
    m_currentFrame.resize(m_width * m_height * 3, 0);
    m_finishedLevel = false;
    m_lastFrameT = m_extraT = 0;
    m_afterEndExtraTime = 0.f;
    m_renderer.m_width = m_width;
    m_renderer.m_height = m_height;
    m_renderer.Begin();

    auto GM = GameManager::sharedState();
    auto PL = GM->getPlayLayer();

    auto songFile = PL->m_level->getAudioFileName();
    auto fadeIn = false;//PL->m_pLevelSettings->m_fadeIn;
    auto fadeOut = false;//PL->m_pLevelSettings->m_fadeOut;
    auto bgVolume = GM->m_fBackgroundMusicVolume;
    auto sfxVolume = GM->m_fEffectsVolume;
    auto isTestMode = PL->m_isTestMode;
    auto songOffset = m_songStartOffset;

    if(PL->m_level->m_nSongID == 0)
        songFile = CCFileUtils::sharedFileUtils()->fullPathForFilename(songFile.c_str(), false);


    std::thread([&, path, songFile, fadeIn, fadeOut, bgVolume, sfxVolume, isTestMode, songOffset]() {
        std::stringstream stream;

        stream << '"' << m_ffmpegPath << '"' << " -y -f rawvideo -pix_fmt rgb24 -s " << m_width << "x" << m_height << " -r " << m_fps << " -i - ";
        if(!m_codec.empty())  stream << "-c:v " << m_codec << " ";
        if(!m_bitrate.empty()) stream << "-b:v " << m_bitrate << " ";
        if(!m_extraArgs.empty()) stream << m_extraArgs << " ";
        else stream << "-pix_fmt yuv420p ";
        stream << "-vf \"vflip\" -an \"" << path << "\" "; // i hope just putting it in "" escapes it

        std::cout << "executing: " << stream.str() << std::endl;

        auto process = subprocess::Popen(stream.str());
        while (m_recording || m_frameHasData) {
            m_lock.lock();
            if(m_frameHasData) {
                const auto frame = m_currentFrame;
                m_frameHasData = false;
                m_lock.unlock();
                process.m_stdin.write(frame.data(), frame.size());
            } else m_lock.unlock();
        }
        if(process.close()) {
            std::cout << "ffmpeg errored :(" << std::endl;
            return;
        }
        std::cout << "video should be done now" << std::endl;

        if(!m_includeAudio || !std::filesystem::exists(songFile)) return;
        wchar_t buffer[MAX_PATH];
        if(!GetTempFileNameW(widen(std::filesystem::temp_directory_path().string()).c_str(), L"rec", 0, buffer)) {
            std::cout << "error getting temp file" << std::endl;
            return;
        }
        auto tempPath = narrow(buffer) + "." + std::filesystem::path(path).filename().string();
        std::filesystem::rename(buffer, tempPath);
        auto totalTime = m_lastFrameT;
        {
            std::stringstream stream;
            stream << '"' << m_ffmpegPath << '"' << " -y -ss " << songOffset << " -i \"" << songFile
            << "\" -i \"" << path << "\" -t " << totalTime << " -c:v copy ";
            if (!m_extraAudioArgs.empty())
                stream << m_extraAudioArgs << " ";
            stream << "-filter:a \"volume=1[0:a]";
            if (fadeIn && !isTestMode)
                stream << ";[0:a]afade=t=in:d=2[0:a]";
            if (fadeOut && m_finishedLevel)
                stream << ";[0:a]afade=t=out:d=2:st=" << (totalTime - m_afterEndDuration - 3.5f) << "[0:a]";
            std::cout << "in " << fadeIn << " out " << fadeOut << std::endl;
            stream << "\" \"" << tempPath << "\"";
            std::cout << "executing: " << stream.str() << std::endl;
            auto process = subprocess::Popen(stream.str());
            if (process.close()) {
                std::cout << "oh god adding the song went wrong cmon" << std::endl;
                return;
            }
        }
        std::filesystem::remove(widen(path));
        std::filesystem::rename(tempPath, widen(path));
        std::cout << "video + audo should be done now!" << std::endl;
    }).detach();
}

void Recorder::Stop() {
    m_renderer.End();
    m_recording = false;
}

void Recorder::captureFrame() {
    while(m_frameHasData) {}
    m_renderer.Capture(m_lock, m_currentFrame, m_frameHasData);
}

void Recorder::handleRecording(PlayLayer* PL, float dt) {
    if(!PL->m_hasLevelCompleteMenu || m_afterEndExtraTime < m_afterEndDuration) {
        if(PL->m_hasLevelCompleteMenu) {
            m_afterEndExtraTime += dt;
            m_finishedLevel = true;
        }
        auto frame_dt = 1. / static_cast<double>(m_fps);
        auto time = PL->m_time + m_extraT - m_lastFrameT;
        if(time >= frame_dt) {
            FMODAudioEngine::sharedEngine()->setBackgroundMusicTime(static_cast<float>(PL->m_time) + m_songStartOffset);
            m_extraT = time - frame_dt;
            m_lastFrameT = PL->m_time;
            captureFrame();
        }
    } else {
        Stop();
    }
}

void Recorder::updateSongOffset(PlayLayer* PL) {
    m_songStartOffset = PL->m_pLevelSettings->m_songStartOffset + PL->timeForXPos2(
        PL->m_pPlayer1->m_position.x, PL->m_isTestMode
    );
}

void MyRenderTexture::Begin() {
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &m_old_fbo);

    m_texture = new CCTexture2D;
    {
        auto data = malloc(m_width * m_height * 3);
        memset(data, 0, m_width * m_height * 3);
        m_texture->initWithData(data, kCCTexture2DPixelFormat_RGB888, m_width, m_height, CCSize(static_cast<float>(m_width), static_cast<float>(m_height)));
        free(data);
    }

    glGetIntegerv(GL_RENDERBUFFER_BINDING_EXT, &m_old_rbo);

    glGenFramebuffersEXT(1, &m_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture->getName(), 0);

    m_texture->setAliasTexParameters();

    m_texture->autorelease();

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_old_rbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_old_fbo);
}

void MyRenderTexture::Capture(std::mutex& lock, std::vector<u8>& data, volatile bool& lul) {
    glViewport(0, 0, m_width, m_height);

    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &m_old_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

    auto director = CCDirector::sharedDirector();
    auto scene = director->getRunningScene();
    scene->visit();

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    lock.lock();
    lul = true;
    glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    lock.unlock();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_old_fbo);
    director->setViewport();
}

void MyRenderTexture::End() {
    m_texture->release();
}

