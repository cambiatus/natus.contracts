#include "natus.hpp"

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
                      std::string location, std::string country, std::string ranking){};

void natus::upsertsrv(std::uint32_t id, std::uint32_t ppa_id, std::uint32_t harvest_id,
                      std::string category, std::string subcategory, float value){};

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