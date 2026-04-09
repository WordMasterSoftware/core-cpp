#pragma once

// 预编译头中只放稳定且高频包含的头文件。
// 这样可以减少重复编译成本，尤其是第三方库头较重时能明显提升增量构建速度。
// 需要注意：经常变化的业务头不适合放在这里，否则会导致预编译头频繁失效，反而拖慢编译。

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <condition_variable>
#include <cstdint>
#include <ctime>
#include <exception>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <asio.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
