__uint128_t combine_ids(std::uint64_t const &x, std::uint64_t const &y)
{
    uint128_t times = 1;
    while (times <= y)
        times *= 10;

    return (x * times) + y;
}

std::uint64_t gen_uuid(const std::uint64_t &x, const std::uint64_t &y)
{
    uint128_t res = combine_ids(x, y);
    return std::hash<uint128_t>{}(res);
}

std::uint64_t hash_to_uint64(eosio::checksum256 hash)
{
    auto hashBytes = hash.extract_as_byte_array().data();
    uint128_t num = combine_ids(
        combine_ids(hashBytes[0], hashBytes[1]),
        combine_ids(hashBytes[2], hashBytes[3]));

    return std::hash<uint128_t>{}(num);
}

std::string uint64_to_str(const std::uint64_t &value)
{
    const char *digits = "0123456789";

    std::string result;
    result.reserve(20);
    std::uint64_t helper = value;

    do
    {
        result += digits[helper % 10];
        helper /= 10;
    } while (helper);

    std::reverse(result.begin(), result.end());

    return result;
}

std::vector<std::string> split(std::string str, std::string delim)
{
    std::vector<std::string> result;
    while (str.size())
    {
        int index = str.find(delim);
        if (index != std::string::npos)
        {
            result.push_back(str.substr(0, index));
            str = str.substr(index + delim.size());
            if (str.size() == 0)
                result.push_back(str);
        }
        else
        {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}
