#include "natus.hpp"

void natus::setconfig(eosio::symbol natus_symbol, std::string version)
{
    require_auth(_self);

    eosio::check(natus_symbol.is_valid(), "provided symbol is not valid");

    auto default_config = config{natus_symbol, version, eosio::asset(0, natus_symbol), 0, 0, 0};
    auto configs = configs_singleton.get_or_create(_self, default_config);

    configs.version = version;
    configs_singleton.set(configs, _self);
}

void natus::sow(eosio::name name,
                eosio::name issuer,
                bool sellable,
                bool transferable,
                eosio::asset max_supply,
                std::uint32_t issue_days,
                std::string base_uri)
{
    require_auth(_self);

    // Notify issuer
    eosio::check(is_account(issuer), "issuer account doesn't exists");
    require_recipient(issuer);

    // Calculate max issue window
    eosio::time_point_sec issue_window = eosio::time_point_sec(0);
    std::uint32_t issue_seconds = issue_days * 24 * 3600;
    issue_window = eosio::time_point_sec(eosio::current_time_point()) + issue_seconds;

    // Validate Supply information
    eosio::check(max_supply.is_valid(), "invalid max_supply");
    eosio::check(max_supply.symbol.is_valid(), "invalid symbol");
    eosio::check(max_supply.amount >= 0, "max supply must be 0 or greater");
    eosio::check(max_supply.symbol.precision() == 0, "max supply must have precision of 0");

    auto config = configs_singleton.get();
    eosio::check(max_supply.symbol == config.natus_symbol, "max supply symbol must be" + config.natus_symbol.code().to_string());

    eosio::check(base_uri.length() <= 255, "base_uri must be less than 256 characters");

    eosio::asset initial_asset = eosio::asset(0, config.natus_symbol);

    harvest_table harvest(_self, _self.value);
    auto existing_harvest = harvest.find(name.value);

    if (existing_harvest == harvest.end())
    {
        harvest.emplace(_self, [&](auto &h) {
            h.name = name;
            h.issuer = issuer;
            h.sellable = sellable;
            h.transferable = transferable;
            h.max_supply = max_supply;
            h.current_supply = initial_asset;
            h.issued_supply = initial_asset;
            h.available_window = issue_window;
            h.base_uri = base_uri;
            h.created_at = eosio::current_time_point();
        });
    }
    else
    {
        harvest.modify(existing_harvest, _self, [&](auto &h) {
            h.name = name;
            h.sellable = sellable;
            h.transferable = transferable;
            h.available_window = issue_window;
            h.base_uri = base_uri;
        });
    }
}

void natus::issue(eosio::name to,
                  eosio::name ppa,
                  eosio::name harvest,
                  eosio::asset quantity,
                  std::string report_hash){

    // TODO: Make sure to use last_serial_number from stats
    // require_auth(_self);

    // eosio::check(is_account(to), "to account doesn't exist");
    // eosio::check(is_account(owner), "owner account doesn't exist");

    // ppa_table ppa(_self, _self.value);
    // auto itr_ppa = ppa.find(ppa_id);
    // eosio::check(itr_ppa != ppa.end(), "cant find PPA with given ppa_id");

    // // Validate Harvest
    // harvest_table harvest(_self, _self.value);
    // auto itr_harvest = harvest.find(harvest_id);
    // eosio::check(itr_harvest != harvest.end(), "cant find harvest with given harvest_id");

    // units_table units(_self, _self.value));
};

void natus::plant(std::uint64_t id, eosio::name owner){};

void natus::transfer(eosio::name from, eosio::name to, std::uint64_t unit_id, std::string memo)
{
    // eosio::check(from != to, "cannot transfer to self");
    // eosio::check(is_account(to), "destination account doesn't exists");

    // require_auth(from);

    // // Check if Natus Unit exists
    // natus::units_table units(_self, _self.value);
    // const auto &found_unit = units.get(unit_id, "can't find a Natus Unit with given unit_id");

    // units.modify(found_unit, _self, [&](auto &u) {
    //     u.owner = to;
    // });

    // // TODO: Update Natus to change owner
}

void natus::upsertppa(std::uint64_t id,
                      eosio::name owner,
                      std::string name,
                      std::string biome,
                      std::string location,
                      std::string country,
                      std::string ranking)
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
            p.id = get_available_id("ppas");
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

void natus::upsertsrv(std::uint64_t id,
                      std::uint64_t ppa_id,
                      eosio::name harvest,
                      std::string category,
                      std::string subcategory,
                      double value)
{
    require_auth(_self);

    // Validate PPA
    ppa_table ppa(_self, _self.value);
    auto itr_ppa = ppa.find(ppa_id);
    eosio::check(itr_ppa != ppa.end(), "cant find PPA with given ppa_id");

    // Validate Harvest
    harvest_table harvests(_self, _self.value);
    auto itr_harvest = harvests.find(harvest.value);
    eosio::check(itr_harvest != harvests.end(), "cant find harvest with given harvest_id");

    // Validate Category
    bool is_category_valid = category == "water" || category == "biodiversity" || category == "carbon";
    eosio::check(is_category_valid, "invalid value for category, must be one of the following: `water`, `biodiversity` or `carbon`");

    // Validate Subcategory
    if (category == "water")
    {
        eosio::check(subcategory == "spring" || subcategory == "course", "invalid subcategory for category 'water");
    }

    if (category == "biodiversity")
    {
        eosio::check(subcategory == "vegetation" || subcategory == "species" || subcategory == "hotspot", "invalid subcategory for category 'biodiversity'");
    }

    if (category == "carbon")
    {
        eosio::check(subcategory == "stock", "invalid subcategory for category 'carbon'");
    }

    ecoservices_table ecoservices(_self, _self.value);
    if (id == 0)
    {
        ecoservices.emplace(_self, [&](auto &e) {
            e.id = get_available_id("ecoservices");
            e.ppa_id = ppa_id;
            e.harvest = harvest;
            e.category = category;
            e.subcategory = subcategory;
            e.value = value;
        });
    }
    else
    {
        auto itr = ecoservices.find(id);
        eosio::check(itr != ecoservices.end(), "cannot find ecoservice with given ID");
        ecoservices.modify(itr, _self, [&](auto &e) {
            e.ppa_id = ppa_id;
            e.harvest = harvest;
            e.category = category;
            e.subcategory = subcategory;
            e.value = value;
        });
    }
};

void natus::clean(std::string t)
{
    require_auth(_self);

    eosio::check(t == "units" ||
                     t == "ecoservices" ||
                     t == "harvest" ||
                     t == "ppa" ||
                     t == "config" ||
                     t == "indexes",
                 "invalid value");

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

    if (t == "config")
    {
        configs_singleton.remove();
    }

    if (t == "indexes")
    {
        _checkconfig();

        auto config = configs_singleton.get();

        config.last_serial_number = 0;
        config.last_ppa_id = 0;
        config.last_ecoservice_id = 0;

        configs_singleton.set(config, _self);
    }
}

// Private

void natus::_checkconfig()
{

    eosio::check(configs_singleton.exists(), "No configuration found, please setup the contract");
}

std::vector<std::string> natus::split(std::string str, std::string delim)
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

// Get available key
uint64_t natus::get_available_id(std::string table)
{
    eosio::check(table == "serial" || table == "ppas" || table == "ecoservices", "Table index not available");

    _checkconfig();
    auto config = configs_singleton.get();

    std::uint64_t new_id = 1;

    if (table == "serial")
    {
        new_id = config.last_serial_number + 1;
        config.last_serial_number = new_id;
        configs_singleton.set(config, _self);

        return new_id;
    }
    else if (table == "ppas")
    {
        new_id = config.last_ppa_id + 1;
        config.last_ppa_id = new_id;
        configs_singleton.set(config, _self);

        return new_id;
    }
    else if (table == "ecoservices")
    {
        new_id = config.last_ecoservice_id + 1;
        config.last_ecoservice_id = new_id;
        configs_singleton.set(config, _self);

        return new_id;
    }

    return new_id;
}
