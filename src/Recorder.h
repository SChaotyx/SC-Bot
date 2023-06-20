#pragma once

#include "includes.h"
#include "../libs/Subprocess.h"
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <string>

using u8 = uint8_t;

class MyRenderTexture {
public:
    unsigned m_width, m_height;
    int m_old_fbo, m_old_rbo;
    unsigned m_fbo;
    CCTexture2D* m_texture;
    void Begin();
    void End();
    void Capture(std::mutex&, std::vector<u8>&, volatile bool&);
};

class Recorder {
public:
    Recorder();
    std::vector<u8> m_currentFrame;
    volatile bool m_frameHasData;
    std::mutex m_lock;
    MyRenderTexture m_renderer;
    unsigned m_width, m_height;
    unsigned m_fps;
    bool m_recording = false;
    double m_lastFrameT, m_extraT;
    bool m_untilEnd = true;
    std::string m_codec = "", m_bitrate = "30M", m_extraArgs = "", m_extraAudioArgs = "";
    float m_afterEndDuration = 3.f;
    float m_afterEndExtraTime;
    float m_songStartOffset;
    bool m_finishedLevel;
    bool m_includeAudio = true;
    std::string m_ffmpegPath = "ffmpeg";

    void Start(const std::string&);
    void Stop();
    void captureFrame();
    void handleRecording(PlayLayer*, float);
    void updateSongOffset(PlayLayer*);
    static void patchMemory(void*, std::vector<uint8_t>);
    std::string videoPath;
};
