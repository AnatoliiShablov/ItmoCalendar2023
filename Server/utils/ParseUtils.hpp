#ifndef ITMOCALENDAR2023_PARSEUTILS_HPP
#define ITMOCALENDAR2023_PARSEUTILS_HPP

#include <algorithm>
#include <charconv>
#include <chrono>
#include <string_view>
#include <variant>

constexpr std::optional<std::chrono::year_month_day> getDate(std::string_view line) noexcept {
    constexpr std::string_view datePattern = "dd.dd.dddd";
    if (!std::equal(line.begin(), line.end(), datePattern.begin(), datePattern.end(), [](char c, char p) {
            return (c == '.' && p == '.') || (std::isdigit(static_cast<unsigned char>(c)) && p == 'd');
        })) {
        return std::nullopt;
    }

    auto toDig = [line](std::size_t pos) -> std::uint32_t {
        return static_cast<int>(line[pos]) - static_cast<int>('0');
    };

    std::chrono::year_month_day result{
        std::chrono::year{static_cast<int>(toDig(6) * 1000 + toDig(7) * 100 + toDig(8) * 10 + toDig(9) * 1)},
        std::chrono::month{toDig(3) * 10 + toDig(4) * 1}, std::chrono::day{toDig(0) * 10 + toDig(1) * 1}};

    return result.ok() && (static_cast<int>(result.year()) > 1970) ? std::optional<decltype(result)>(result)
                                                                   : std::nullopt;
}

constexpr std::optional<std::chrono::minutes> getTime(std::string_view line) noexcept {
    constexpr std::string_view datePattern = "dd:dd";
    if (!std::equal(line.begin(), line.end(), datePattern.begin(), datePattern.end(), [](char c, char p) {
            return (c == ':' && p == ':') || (std::isdigit(static_cast<unsigned char>(c)) && p == 'd');
        })) {
        return std::nullopt;
    }

    auto toDig = [line](std::size_t pos) -> std::uint32_t {
        return static_cast<int>(line[pos]) - static_cast<int>('0');
    };

    std::chrono::hours h{toDig(0) * 10 + toDig(1) * 1};
    std::chrono::minutes m{toDig(3) * 10 + toDig(4) * 1};

    return (h.count() < 24 && m.count() < 60) ? std::optional<std::chrono::minutes>{h + m} : std::nullopt;
}

constexpr std::optional<std::optional<std::uint32_t>> getOffset(std::string_view line) {
    if (line == "нет") {
        return std::optional<std::uint32_t>{};
    }

    std::uint32_t value;

    if (auto res = std::from_chars(reinterpret_cast<char const*>(line.data()),
                                   reinterpret_cast<char const*>(line.data() + line.size()), value);
        res.ec != std::errc{} || res.ptr != reinterpret_cast<char const*>(line.data() + line.size()) || value < 0) {
        return std::nullopt;
    }

    return value;
}

#endif  // ITMOCALENDAR2023_PARSEUTILS_HPP
