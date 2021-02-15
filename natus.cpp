#include "natus.hpp"

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

void natus::issue(eosio::name to, eosio::name owner, std::string origin,
                  std::string harvest, std::string report_hash){};

void natus::plant(std::uint32_t id, eosio::name owner){};

void natus::transfer(eosio::name from, eosio::name to, std::uint32_t unit_id, std::string memo)
{
    eosio::check(from != to, "cannot transfer to self");
    eosio::check(is_account(to), "destination account doesn't exists");

    require_auth(from);

    // Check if Natus Unit exists
    natus::units_table units(_self, _self.value);
    const auto &found_unit = units.get(unit_id, "can't find a Natus Unit with given unit_id");

    units.modify(found_unit, _self, [&](auto &u) {
        u.owner = to;
    });

    // TODO: What happens with scope when changing owners?

    // TODO: Update Natus to change owner
}

void natus::upsertppa(std::uint32_t id, eosio::name owner, std::string name, std::string biome,
                      std::string location, std::string country, std::string ranking)
{
    require_auth(_self);

    // Value validations
    eosio::check(is_account(owner), "owner account doesn't exist");
    eosio::check(name.length() <= 256, "invalid length for name, must be under 255 chars");
    bool is_biome_valid = biome == "pantanal" || biome == "atlanticforest" || biome == "amazonrainforest" || biome == "caatinga";
    eosio::check(is_biome_valid, "invalid value for biome, must be one of the following: `pantanal`, `atlanticforest`, `amazonrainforest`, `caatinga`");

    // Validate location
    std::vector<std::string> latlon_split = split(location, ",");
    eosio::check(latlon_split.size() == 2, "cant parse location, must be formated like: 0.000000,-0.000000");
    eosio::check(latlon_split[0].length() >= 8 && latlon_split[0].length() <= 10, "cant parse location latitude");
    eosio::check(latlon_split[1].length() >= 8 && latlon_split[1].length() <= 10, "cant parse location longitude");

    bool is_country_valid = country == "brazil";
    eosio::check(is_country_valid, "invalid value for country must be one of the following:  `brazil`");

    eosio::check(ranking.length() <= 256, "invalid length for ranking, must be under 255 chars");

    ppa_table ppa(_self, _self.value);

    if (id == 0)
    {
        ppa.emplace(_self, [&](auto &p) {
            p.id = ppa.available_primary_key() == 0 ? 1 : ppa.available_primary_key();
            p.name = name;
            p.biome = biome;
            p.location = location;
            p.country = country;
            p.ranking = ranking;
        });
    }
    else
    {
        auto itr = ppa.find(id);
        eosio::check(itr != ppa.end(), "cannot find PPA with given ID");
        ppa.modify(itr, _self, [&](auto &p) {
            p.name = name;
            p.location = location;
            p.country = country;
            p.ranking = ranking;
        });
    }
};

void natus::upsertsrv(std::uint32_t id, std::uint32_t ppa_id, std::uint32_t harvest_id,
                      std::string category, std::string subcategory, float value)
{
    require_auth(_self);
};

void natus::upserthrvst(std::uint32_t id, std::uint32_t year, std::string name)
{
    require_auth(_self);

    eosio::check(year >= 2021, "year invalid, must be at least 2021");
    eosio::check(name.length() <= 256, "invalid length for the name, must be under 255 chars");

    harvest_table harvest(_self, _self.value);

    if (id == 0)
    {
        harvest.emplace(_self, [&](auto &h) {
            h.id = harvest.available_primary_key() == 0 ? 1 : harvest.available_primary_key();
            h.year = year;
            h.name = name;
        });
    }
    else
    {
        auto itr = harvest.find(id);
        eosio::check(itr != harvest.end(), "cannot find harvest with given ID");
        harvest.modify(itr, _self, [&](auto &h) {
            h.year = year;
            h.name = name;
        });
    }
}

void natus::clean(std::string t)
{
    require_auth(_self);

    eosio::check(t == "units" || t == "ecoservices" || t == "harvest" || t == "ppa", "invalid value");

    if (t == "units")
    {
        natus::units_table units(_self, _self.value);
        for (auto itr = units.begin(); itr != units.end();)
        {
            itr = units.erase(itr);
        }
    }

    if (t == "ecoservices")
    {
        natus::ecoservices_table services(_self, _self.value);
        for (auto itr = services.begin(); itr != services.end();)
        {
            itr = services.erase(itr);
        }
    }

    if (t == "harvest")
    {
        natus::harvest_table harvest(_self, _self.value);
        for (auto itr = harvest.begin(); itr != harvest.end();)
        {
            itr = harvest.erase(itr);
        }
    }

    if (t == "ppa")
    {
        natus::ppa_table ppa(_self, _self.value);
        for (auto itr = ppa.begin(); itr != ppa.end();)
        {
            itr = ppa.erase(itr);
        }
    }
}
