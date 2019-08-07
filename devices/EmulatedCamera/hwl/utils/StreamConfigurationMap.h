/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef EMULATOR_STREAM_CONFIGURATION_MAP_H_
#define EMULATOR_STREAM_CONFIGURATION_MAP_H_

#include "hwl_types.h"
#include "system/camera_metadata.h"
#include <unordered_map>
#include <set>
#include "utils/Timers.h"

namespace android {

using google_camera_hal::HalCameraMetadata;

typedef std::pair<uint32_t, uint32_t> StreamSize;
typedef std::pair<android_pixel_format_t, StreamSize> StreamConfig;

inline bool operator == (const StreamConfig& lhs, const StreamConfig& rhs) {
    return (std::get<0> (lhs) == std::get<0> (rhs)) &&
        (std::get<1>(lhs).first == std::get<1>(rhs).first) &&
        (std::get<1>(lhs).second == std::get<1>(rhs).second);
}

struct StreamConfigurationHash {
    inline std::size_t operator() (const StreamConfig& entry) const {
        size_t result = 1;
        size_t hashValue = 31;
        result = hashValue*result + std::hash<android_pixel_format_t> {} (std::get<0>(entry));
        result = hashValue*result + std::hash<uint32_t> {} (std::get<1>(entry).first);
        result = hashValue*result + std::hash<uint32_t> {} (std::get<1>(entry).second);
        return result;
    }
};

class StreamConfigurationMap {
public:

    StreamConfigurationMap(const HalCameraMetadata& chars);

    const std::set<android_pixel_format_t>& getOutputFormats() const {
        return mStreamOutputFormats;
    }

    const std::set<StreamSize>& getOutputSizes(android_pixel_format_t format) {
        return mStreamOutputSizeMap[format];
    }

    nsecs_t getOutputMinFrameDuration(StreamConfig configuration) const {
        auto ret = mStreamMinDurationMap.find(configuration);
        return (ret == mStreamMinDurationMap.end()) ? 0 : ret->second;
    }

    nsecs_t getOutputStallDuration(StreamConfig configuration) const {
        auto ret = mStreamStallMap.find(configuration);
        return (ret == mStreamStallMap.end()) ? 0 : ret->second;
    }

    bool supportsReprocessing() const {
        return !mStreamInputOutputMap.empty();
    }

    const std::set<android_pixel_format_t>& getValidOutputFormatsForInput(
            android_pixel_format_t format) {
        return mStreamInputOutputMap[format];
    }

    const std::set<android_pixel_format_t>& getInputFormats() {
        return mStreamInputFormats;
    }

private:

    void appendAvailableStreamConfigurations(const camera_metadata_ro_entry& entry);
    void appendAvailableStreamMinDurations(const camera_metadata_ro_entry_t& entry);
    void appendAvailableStreamStallDurations(const camera_metadata_ro_entry& entry);

    const size_t kStreamFormatOffset = 0;
    const size_t kStreamWidthOffset = 1;
    const size_t kStreamHeightOffset = 2;
    const size_t kStreamIsInputOffset = 3;
    const size_t kStreamMinDurationOffset = 3;
    const size_t kStreamStallDurationOffset = 3;
    const size_t kStreamConfigurationSize = 4;

    std::set<android_pixel_format_t> mStreamOutputFormats;
    std::unordered_map<android_pixel_format_t, std::set<StreamSize>> mStreamOutputSizeMap;
    std::unordered_map<StreamConfig, nsecs_t, StreamConfigurationHash> mStreamStallMap;
    std::unordered_map<StreamConfig, nsecs_t, StreamConfigurationHash> mStreamMinDurationMap;
    std::set<android_pixel_format_t> mStreamInputFormats;
    std::unordered_map<android_pixel_format_t, std::set<android_pixel_format_t>>
        mStreamInputOutputMap;
};


} // namespace android ends here

#endif  // EMULATOR_STREAM_CONFIGURATION_MAP_H_
