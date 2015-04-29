/*
 *  VideoEncoderX264or5 - Base class for VideoEncoderX264 and VideoEncoderX265
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of media-streamer.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:  Marc Palau <marc.palau@i2cat.net>
 *            David Cassany <david.cassany@i2cat.net>
 */

#include "VideoEncoderX264or5.hh"

VideoEncoderX264or5::VideoEncoderX264or5(FilterRole fRole_, bool sharedFrames, int framerate) :
OneToOneFilter(false, fRole_, sharedFrames), annexB(false), forceIntra(false), bitrate(0), gop(0), threads(0), needsConfig(false)
{
    fType = VIDEO_ENCODER;

    if (framerate <= 0){
        fps = VIDEO_DEFAULT_FRAMERATE;
    } else {
        fps = framerate;
    }

    setFrameTime(std::chrono::nanoseconds(std::nano::den/fps));
    midFrame = av_frame_alloc();

    initializeEventMap();
}

VideoEncoderX264or5::~VideoEncoderX264or5()
{
    if (midFrame){
        av_frame_free(&midFrame);
    }
}

bool VideoEncoderX264or5::doProcessFrame(Frame *org, Frame *dst) 
{
    VideoFrame* rawFrame = dynamic_cast<VideoFrame*> (org);
    VideoFrame* codedFrame = dynamic_cast<VideoFrame*> (dst);

    if (!rawFrame || !codedFrame) {
        utils::errorMsg("Error encoding video frame: org and dst MUST be VideoFrame");
        return false;
    }

    if (!reconfigure(rawFrame, codedFrame)) {
        return false;
    }

    if (!fill_x264or5_picture(rawFrame)){
        utils::errorMsg("Could not fill x264_picture_t from frame");
        return false;
    }

    if (!encodeFrame(codedFrame)) {
        utils::errorMsg("Could not encode video frame");
        return false;
    }

    return true;
}

bool VideoEncoderX264or5::fill_x264or5_picture(VideoFrame* videoFrame)
{
    if (avpicture_fill((AVPicture *) midFrame, videoFrame->getDataBuf(),
            (AVPixelFormat) libavInPixFmt, videoFrame->getWidth(),
            videoFrame->getHeight()) <= 0){
        utils::errorMsg("Could not feed AVFrame");
        return false;
    }

    if (!fillPicturePlanes(midFrame->data, midFrame->linesize)) {
        utils::errorMsg("Could not fill picture planes");
        return false;
    }

    return true;
}

bool VideoEncoderX264or5::configure(int bitrate_, int fps_, int gop_, int lookahead_, int threads_, bool annexB_, std::string preset_)
{
    bitrate = bitrate_;
    gop = gop_;
    lookahead = lookahead_;
    threads = threads_;
    annexB = annexB_;
    preset = preset_;

    if (fps_ <= 0) {
        fps = VIDEO_DEFAULT_FRAMERATE;
    } else {
        fps = fps_;
    }

    setFrameTime(std::chrono::nanoseconds(std::nano::den/fps));
    needsConfig = true;
    return true;
}

void VideoEncoderX264or5::configEvent(Jzon::Node* params, Jzon::Object &outputNode)
{
    int tmpBitrate; 
    int tmpFps; 
    int tmpGop; 
    int tmpLookahead; 
    int tmpThreads; 
    bool tmpAnnexB; 
    std::string tmpPreset;

    if (!params) {
        outputNode.Add("error", "Error configuring video encoder. Params node not found");
        return;
    }

    tmpBitrate = bitrate; 
    tmpFps = fps; 
    tmpGop = gop; 
    tmpLookahead = lookahead; 
    tmpThreads = threads; 
    tmpAnnexB = annexB; 
    tmpPreset = preset;

    if (params->Has("bitrate")) {
        tmpBitrate = params->Get("bitrate").ToInt();
    }

    if (params->Has("fps")) {
        tmpFps = params->Get("fps").ToInt();
    }

    if (params->Has("gop")) {
        tmpGop = params->Get("gop").ToInt();
    }

    if (params->Has("lookahead")) {
        tmpLookahead = params->Get("lookahead").ToInt();
    }

    if (params->Has("threads")) {
        tmpThreads = params->Get("threads").ToInt();
    }

    if (params->Has("annexb")) {
        tmpAnnexB = params->Get("annexb").ToBool();
    }

    if (params->Has("preset")) {
        tmpPreset = params->Get("preset").ToString();
    }

    if (!configure(tmpBitrate, tmpFps, tmpGop, tmpLookahead, tmpThreads, tmpAnnexB, tmpPreset)) {
        outputNode.Add("error", "Error configuring video encoder");
    } else {
        outputNode.Add("error", Jzon::null);
    }
}

void VideoEncoderX264or5::forceIntraEvent(Jzon::Node* params)
{
    forceIntra = true;
}

void VideoEncoderX264or5::initializeEventMap()
{
    eventMap["forceIntra"] = std::bind(&VideoEncoderX264or5::forceIntraEvent, this, std::placeholders::_1);
    eventMap["configure"] = std::bind(&VideoEncoderX264or5::configEvent, this, std::placeholders::_1, std::placeholders::_2);
}

void VideoEncoderX264or5::doGetState(Jzon::Object &filterNode)
{
    filterNode.Add("gop", std::to_string(gop));
    filterNode.Add("bitrate", std::to_string(bitrate));
    filterNode.Add("threads", std::to_string(threads));
    filterNode.Add("fps", std::to_string(fps));
}
