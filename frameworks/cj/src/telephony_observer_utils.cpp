/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "telephony_observer_utils.h"

namespace OHOS {
namespace Telephony {
    char* MallocCString(const std::string& origin)
    {
        if (origin.empty()) {
            return nullptr;
        }
        auto lenth = origin.length() + 1;
        char* res = static_cast<char*>(malloc(sizeof(char) * lenth));
        if (res == nullptr) {
            return nullptr;
        }
        return std::char_traits<char>::copy(res, origin.c_str(), lenth);
    }

    std::string ToUtf8(std::u16string str16)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        // 将std::u16string转换为std::string
        std::string result = convert.to_bytes(u16str);
        return result;
    }
}
}