#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../mediainfo/container.hpp"

namespace porla::MediaInfo
{
NLOHMANN_JSONIFY_ALL_THINGS(
    StreamAudio,
    codec_id,
    duration,
    format);

NLOHMANN_JSONIFY_ALL_THINGS(
    StreamGeneral,
    bitrate_overall,
    codec_id,
    duration,
    format,
    format_profile);

NLOHMANN_JSONIFY_ALL_THINGS(
    StreamVideo,
    aspect_ratio_display,
    aspect_ratio_pixel,
    bitrate,
    chroma_subsampling,
    codec_id,
    color_space,
    duration,
    format,
    format_level,
    format_profile,
    format_tier,
    frame_rate,
    hdr_format,
    hdr_format_level,
    hdr_format_profile,
    height,
    width);

NLOHMANN_JSONIFY_ALL_THINGS(
    Container,
    general,
    audio_streams,
    video_streams);
}