#include "natus.hpp"
#include "utils/utils.cpp"

void natus::setconfig(eosio::symbol natus_symbol, std::string version)
{
    require_auth(_self);

    eosio::check(natus_symbol.is_valid(), "provided symbol is not valid");

    auto default_config = config{natus_symbol, version, eosio::asset(0, natus_symbol), 0, 0};
    auto configs = configs_singleton.get_or_create(_self, default_config);

    configs.version = version;
    configs_singleton.set(configs, _self);
}

void natus::upsertcollec(eosio::name name,
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

    collection_table collection(_self, _self.value);
    auto existing_collection = collection.find(name.value);

    if (existing_collection == collection.end())
    {
        collection.emplace(_self, [&](auto &co)
                           {
                               co.name = name;
                               co.issuer = issuer;
                               co.sellable = sellable;
                               co.transferable = transferable;
                               co.max_supply = max_supply;
                               co.current_supply = initial_asset;
                               co.issued_supply = initial_asset;
                               co.available_window = issue_window;
                               co.base_uri = base_uri;
                               co.created_at = eosio::current_time_point();
                           });
    }
    else
    {
        collection.modify(existing_collection, _self, [&](auto &co)
                          {
                              co.name = name;
                              co.sellable = sellable;
                              co.transferable = transferable;
                              co.available_window = issue_window;
                              co.base_uri = base_uri;
                          });
    }
}

void natus::upsertreport(eosio::name collection, std::uint64_t ppa_id, std::string report_hash)
{
    require_auth(_self);

    eosio::check(report_hash.length() > 0, "report_hash is empty");

    auto ppa_collection_id = gen_uuid(ppa_id, collection.value);
    reports_table reports(_self, _self.value);
    auto report = reports.find(ppa_collection_id);

    if (report == reports.end())
    {
        reports.emplace(_self, [&](auto &r)
                        {
                            r.id = ppa_collection_id;
                            r.collection = collection;
                            r.ppa_id = ppa_id;
                            r.report_hash = report_hash;
                        });
    }
    else
    {
        reports.modify(report, _self, [&](auto &r)
                       { r.report_hash = report_hash; });
    }
}

void natus::issue(eosio::name to,
                  std::uint64_t ppa_id,
                  eosio::name collection,
                  eosio::asset quantity,
                  std::string memo)
{
    eosio::check(memo.size() <= 256, "memo has more than 256 bytes");
    eosio::check(is_account(to), "to account doesn't exist");

    auto config = configs_singleton.get();

    // Validate quantity
    eosio::check(quantity.is_valid(), "invalid quantity");
    eosio::check(quantity.amount > 0, "quantity must be positive");
    eosio::check(quantity.symbol == config.natus_symbol, "we only accept " + config.natus_symbol.code().to_string() + " tokens");

    ppa_table ppa(_self, _self.value);
    auto itr_ppa = ppa.find(ppa_id);
    eosio::check(itr_ppa != ppa.end(), "cant find PPA with given ppa_id");

    // Validate Collection
    collection_table c(_self, _self.value);
    const auto &found_collection = c.get(collection.value, "cant find collection with given collection_id");

    eosio::require_auth(found_collection.issuer);

    // If it is configured to have a time window
    if (found_collection.available_window != eosio::time_point_sec(0))
    {
        eosio::check(_now() <= found_collection.available_window, "This collection is closed, cannot issue more");
    }

    auto available_supply = found_collection.max_supply - found_collection.issued_supply;
    eosio::check(quantity.amount <= available_supply.amount, "Cannot issue more than max supply");

    _add_balance(to, quantity, ppa_id, collection);

    // Update collection issued_supply
    c.modify(found_collection, _self, [&](auto &co)
             {
                 co.current_supply += quantity;
                 co.issued_supply += quantity;
             });

    // Update System's total supply
    config = configs_singleton.get();
    config.total_issued_supply += quantity;
    configs_singleton.set(config, _self);
};

void natus::plant(std::uint64_t ppa_id,
                  eosio::name collection_name,
                  eosio::asset quantity,
                  eosio::name owner,
                  std::string memo)
{
    auto config = configs_singleton.get();

    // Validate data, memo, quantity, id
    eosio::check(ppa_id > 0, "invalid ppa_id");
    eosio::check(quantity.is_valid(), "invalid amount");
    eosio::check(quantity.amount > 0, "amount must be positive");
    eosio::check(quantity.symbol == config.natus_symbol, "invalid symbol");
    eosio::check(memo.size() <= 256, "memo has more than 256 bytes");

    // Validate ppa_collection_id
    auto ppa_collection_id = gen_uuid(ppa_id, collection_name.value);
    eosio::check(ppa_collection_id > 0, "invalid ppa_id or collection");

    collection_table collections(_self, _self.value);
    const auto &collection = collections.get(collection_name.value, "cant find collection with given collection_id");

    if (eosio::get_sender() == owner)
    {
        require_auth(owner);
    }
    else
    {
        require_auth(collection.issuer);
    }

    // Check Collection to see if it is still available
    eosio::check(collection.transferable, "this collection doesn't allow transfers");
    eosio::check(_now() <= collection.available_window, "collection over");
    eosio::check(collection.current_supply >= quantity, "supply inconsistent");

    // Make sure the user has the necessary balance for planting
    accounts_table accounts(_self, owner.value);
    auto account = accounts.get(ppa_collection_id, "cant find the proposed balance");
    eosio::check(account.balance.amount >= quantity.amount, "quantity bigger than avaiable balance");

    _sub_balance(owner, quantity, ppa_id, collection_name);

    // Decrease collection
    collections.modify(collection, _self, [&](auto &co)
                       { co.current_supply -= quantity; });
}

void natus::transfer(eosio::name from,
                     eosio::name to,
                     std::uint64_t ppa_id,
                     eosio::name collection,
                     eosio::asset quantity,
                     std::string memo)
{
    require_auth(from);

    eosio::check(from != to, "cannot transfer to self");
    eosio::check(is_account(to), "destination account doesn't exists");
    eosio::check(memo.size() <= 256, "memo has more than 256 bytes");

    // Validate Collection, if its possible to transfer
    collection_table h(_self, _self.value);
    const auto &found_collection = h.get(collection.value, "cant find collection with given collection_id");
    eosio::check(found_collection.transferable, "Collection already closed");

    // Check quantity
    auto config = configs_singleton.get();
    eosio::check(quantity.symbol == config.natus_symbol, "Symbol don't match the system's");
    eosio::check(quantity.amount > 0, "amount must be positive");
    eosio::check(quantity.symbol.precision() == 0, "precision must be 0");
    eosio::check(quantity.is_valid(), "invalid quantity");

    _sub_balance(from, quantity, ppa_id, collection);
    _add_balance(to, quantity, ppa_id, collection);
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
        ppa.emplace(_self, [&](auto &p)
                    {
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
        ppa.modify(itr, _self, [&](auto &p)
                   {
                       p.name = name;
                       p.location = location;
                       p.country = country;
                       p.ranking = ranking;
                   });
    }
};

void natus::upsertsrv(std::uint64_t id,
                      std::uint64_t ppa_id,
                      eosio::name collection,
                      std::string category,
                      std::string subcategory,
                      double value)
{
    require_auth(_self);

    // Validate PPA
    ppa_table ppa(_self, _self.value);
    auto itr_ppa = ppa.find(ppa_id);
    eosio::check(itr_ppa != ppa.end(), "cant find PPA with given ppa_id");

    // Validate Collection
    collection_table collections(_self, _self.value);
    auto itr_collection = collections.find(collection.value);
    eosio::check(itr_collection != collections.end(), "cant find collection with given collection_id");

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
        ecoservices.emplace(_self, [&](auto &e)
                            {
                                e.id = get_available_id("ecoservices");
                                e.ppa_id = ppa_id;
                                e.collection = collection;
                                e.category = category;
                                e.subcategory = subcategory;
                                e.value = value;
                            });
    }
    else
    {
        auto itr = ecoservices.find(id);
        eosio::check(itr != ecoservices.end(), "cannot find ecoservice with given ID");
        ecoservices.modify(itr, _self, [&](auto &e)
                           {
                               e.ppa_id = ppa_id;
                               e.collection = collection;
                               e.category = category;
                               e.subcategory = subcategory;
                               e.value = value;
                           });
    }
};

void natus::clean(std::string t, eosio::name scope)
{
    require_auth(_self);

    eosio::check(
        t == "ecoservices" ||
            t == "collection" ||
            t == "ppa" ||
            t == "config" ||
            t == "indexes" ||
            t == "accounts",
        "invalid value");

    if (t == "ecoservices")
    {
        natus::ecoservices_table services(_self, _self.value);
        for (auto itr = services.begin(); itr != services.end();)
        {
            itr = services.erase(itr);
        }
    }

    if (t == "collection")
    {
        natus::collection_table collection(_self, _self.value);
        for (auto itr = collection.begin(); itr != collection.end();)
        {
            itr = collection.erase(itr);
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

        config.last_ppa_id = 0;
        config.last_ecoservice_id = 0;

        configs_singleton.set(config, _self);
    }

    if (t == "accounts")
    {
        accounts_table accounts(_self, scope.value);
        for (auto itr = accounts.begin(); itr != accounts.end();)
        {
            itr = accounts.erase(itr);
        }
    }
}

// Private

void natus::_checkconfig()
{

    eosio::check(configs_singleton.exists(), "No configuration found, please setup the contract");
}

eosio::time_point_sec natus::_now()
{
    return eosio::time_point_sec(eosio::current_time_point());
}

// Get available key
uint64_t natus::get_available_id(std::string table)
{
    eosio::check(table == "ppas" || table == "ecoservices", "Table index not available");

    _checkconfig();
    auto config = configs_singleton.get();

    std::uint64_t new_id = 1;

    if (table == "ppas")
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

void natus::_add_balance(eosio::name owner,
                         eosio::asset quantity,
                         std::uint64_t ppa_id,
                         eosio::name collection)
{
    auto ppa_collection_id = gen_uuid(ppa_id, collection.value);
    accounts_table accounts(_self, owner.value);
    auto account = accounts.find(ppa_collection_id);

    // Create new balance if there isn't one yet
    if (account == accounts.end())
    {
        accounts.emplace(_self, [&](auto &a)
                         {
                             a.ppa_collection_id = ppa_collection_id;
                             a.balance = quantity;
                             a.ppa_id = ppa_id;
                             a.collection = collection;
                         });
    }
    else
    {
        accounts.modify(account, _self, [&](auto &a)
                        { a.balance += quantity; });
    }
}

void natus::_sub_balance(eosio::name owner,
                         eosio::asset quantity,
                         std::uint64_t ppa_id,
                         eosio::name collection)
{
    auto ppa_collection_id = gen_uuid(ppa_id, collection.value);
    accounts_table accounts(_self, owner.value);
    const auto &account = accounts.get(ppa_collection_id, "token from ppa and collection don't exist in account");

    eosio::check(account.balance.amount >= quantity.amount, "quantity is more than account balance");

    if (account.balance.amount == quantity.amount)
    {
        accounts.erase(account);
    }
    else
    {
        accounts.modify(account, _self, [&](auto &a)
                        { a.balance -= quantity; });
    }
}
