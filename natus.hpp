#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/time.hpp>

/**
 * EOSIO Contract describing Natus Units
 */
class [[eosio::contract("natus")]] natus : public eosio::contract
{
public:
    using contract::contract;

    // Constructor
    natus(eosio::name receiver,
          eosio::name code,
          eosio::datastream<const char *> ds)
        : contract(receiver, code, ds),
          configs_singleton(receiver, receiver.value) {}

    /**
     * Singleton table that holds contract general stats and configurations
     * 
     * * natus_symbol: Natus units symbol
     * * version: Current version of the contract
     * * total_issued_supply: system wide issued Natus Units
     * * last_serial_number: system's last used serial number
     * * last_ppa_id: system's last used ppa_id
     * * last_ecoservice_id: system's last used ecoservice_id
     */
    TABLE config
    {
        eosio::symbol natus_symbol;       // Symbol used to represent Natus Units
        std::string version;              // Current version for the contract
        eosio::asset total_issued_supply; // Total supply of Natus Unit issued, counts planted ones

        // Custom indexing
        std::uint64_t last_serial_number; // Last used serial number
        std::uint64_t last_ppa_id;        // Last used ppa_id
        std::uint64_t last_ecoservice_id; // Last used ecoservice_id
    };

    /**
     * Table that holds information about all Natus Units
     * Natus Unit is a digital good that represents a basket of services provided by a given PPA (see links below) in a given period called `harvest`
     * 
     * Here is a general description of available fields on a Natus Unit:
     * 
     * * id: ID
     * * owner: Current owner 
     * * ppa_origin: PPA name where this unit come from
     * * harvest: related harvest, used on the scope as well
     * * serial_number: serialized ID, increments independetly of harvest
     * * report_hash: Hash for the full report document, used to garantee authenticity of the document
     * * planted_at: Date it was planted, its empty if it is still not planted
     * * inserted_at: Date it was inserted/created
     * * updated_at: Date it was updated
     */
    TABLE units
    {
        // Scope is self
        std::uint64_t serial_number;            // Serial number for external reference
        eosio::name owner;                      // Current Owner
        eosio::name harvest;                    // Harvest where it comes from
        std::uint64_t ppa_id;                   // PPA that created the Unit
        std::optional<std::string> report_hash; // hash for the report, should be used together with harvest.base_uri
        eosio::time_point_sec planted_at;       // Date of planting, if its zero, it still havent been planted yet
        eosio::time_point_sec issued_at;        // Date of issuing
        eosio::time_point_sec updated_at;       // Last update date (transfer and plant change this value)

        std::uint64_t primary_key() const { return serial_number; }
        std::uint64_t get_owner() const { return owner.value; }

        EOSLIB_SERIALIZE(units, (serial_number)(owner)(harvest)(ppa_id)(report_hash)(planted_at)(issued_at)(updated_at));
    };

    /**
     * Table that holds an account balance
     * Natus Units always are created based on a single PPA resourses as a way to represent its ecosystem services.
     * For that reason we use both Harvest and PPA to indentify the origin of the 
     * 
     * So a single user can have multiple Natus Units balance, each one representing the portion from each PPA/Harvest
    */
    TABLE accounts
    {
        // Scope is owner
        uint128_t ppa_harvest_id;

        eosio::asset balance; // Amount of units
        std::uint64_t ppa_id; // PPA that created the Unit
        eosio::name harvest;  // Harvest where those amounted units comes from

        uint128_t primary_key() const { return ppa_harvest_id; }

        EOSLIB_SERIALIZE(accounts, (ppa_harvest_id)(balance)(ppa_id)(harvest));
    };

    /**
     * Harvest is the moment where an PPA generates new Natus Unit to the system
     * 
     * It holds configurations and characteristics of the Harvest.
     * 
     * * id: incremental ID of the harvest
     * * name: name, used as foreign_key and Units scope
     * * sellable: if the units of this harvest can be selled by Natus Foundation. This value can change over time
     * * transferable: if the units can be transferred between users, after a Harvest ends, its no longer possible
     * * max_supply: max possible supply
     * * current_supply: supply in circulation, don't include planted units
     * * issued_supply: supply issued so far, always smaller than max_supply but can be higher than current_supply
     * * issue_window: max date where new units can be issued, after that date it will not be possible to issue new ones
     * * base_uri: base URI for storing the Units certificates
     * * created_at: creation date for the harvest
     */
    TABLE harvests
    {
        // Scope is _self
        eosio::name name;                       // Eg.: "2021.1", "2021.2"
        eosio::name issuer;                     // Issuer account
        bool sellable;                          // Informs if its sellable
        bool transferable;                      // Informs if its transferable
        eosio::asset max_supply;                // Whole harvest assets
        eosio::asset current_supply;            // Supply in circulation, don't include planted units
        eosio::asset issued_supply;             // Everything issued so far
        eosio::time_point_sec available_window; // Max available date (Transfer, Plant or Issue)
        std::string base_uri;                   // Base URI for the Harvest Units
        eosio::time_point_sec created_at;       // Date of creation

        std::uint64_t primary_key() const { return name.value; }

        EOSLIB_SERIALIZE(harvests, (name)(issuer)(sellable)(transferable)(max_supply)(current_supply)(issued_supply)(available_window)(base_uri)(created_at));
    };

    /**
     * Eco services provided by a PPA during a Harvest.
     * It allows for certain services to be informed on a PPA provided services on a given Harvest
     * 
     * Available `category`s: `water`, `carbon` and `biodiversity`
     * Available `subcategory`s: `course`, `spring`, `stock`, `vegetation`, `species`, `hotspot`
     * Value is a double, can be interpreted as tons, numbers, m2, etc
     */
    TABLE ecoservices
    {
        // Scope is _self
        std::uint64_t id;
        std::uint64_t ppa_id;
        eosio::name harvest;
        std::string category;
        std::string subcategory;
        double value;

        std::uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(ecoservices, (id)(ppa_id)(harvest)(category)(subcategory)(value));
    };

    /**
     * Central data structure, represents a PPA
     * 
     * PPAs can exist in different names depending on the country, with different legislations with the same country
     * We rank the PPA based on a criterea on how good they are on preserve the nature, it also affects the initial price offering
     * 
     * PPAs can be updated to change its name, owner and ranking
    */
    TABLE ppas
    {
        // Scope is _self
        std::uint64_t id;
        eosio::name owner;
        std::string name;
        std::string biome;
        std::string location;
        std::string country;
        std::string ranking;

        std::uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(ppas, (id)(owner)(name)(biome)(location)(country)(ranking));
    };

    /**
     * Sow a new harvest or update an existing one. 
     * Harvests usually happen anually but can happen other times in special cases.
     * 
     * Update only changes the following properties: `sellable`, `transferable`, `issue_days`, `base_uri`
     * 
     * Validations:
     * * id: used to update an existing harvest, use 0 to create a new one
     * * name: name of the harvest
     * * issuer: issuer account
     * * sellable: informs if this harvest units are sellable from Natus Foundation
     * * transferable: informs if its possible to transfer Units
     * * max_supply: maximum supply possible on this harvest
     * * issue_days: # of days where the issue will be available 
     * * base_uri: base URI for all certificates for units on this harvest
     */
    ACTION sow(eosio::name name,
               eosio::name issuer,
               bool sellable,
               bool transferable,
               eosio::asset max_supply,
               std::uint32_t issue_days,
               std::string base_uri);

    /**
     * Issue a new Natus Unit
     * 
     * * to: destination EOSIO account, must be valid and exist
     * * ppa: origin PPA ID, must be present in the PPA table
     * * harvest: harvest name, must be present on the Havest table
     * * report_hash: hash of the report corresponding to this PPA's Natus Units
     */
    ACTION issue(eosio::name to,
                 std::uint64_t ppa_id,
                 eosio::name harvest,
                 eosio::asset quantity,
                 std::string report_hash,
                 std::string memo);

    /**
     * Plant a Natus Unit
     * 
     * Validations:
     * * ID: ID of the Natus Unit, must exist and not be planted yet
     * * Owner: Owner EOSIO Account, must be valid and exist. Also should be the owner of the given Natus Unit ID
     */
    ACTION plant(std::uint64_t id,
                 eosio::name owner);

    /**
     * Transfer ownership of a given Natus Unit
     * Natus Units cannot be transferred if its already planted
     * 
     * Validations:
     * * from: EOS account, must exist and need to be owner of the `unit_id` provided
     * * to: EOS account, must exist
     * * unit_id: ID of a Natus Unit
     * * memo: Optional memo max 256 characters
     */
    ACTION transfer(eosio::name from,
                    eosio::name to,
                    std::uint64_t unit_id,
                    std::string memo);

    /**
     * Upsert PPA 
     * Admin only action that insert or update an PPA
     * 
     * Update only allows changes on `name`, `owner` and `ranking`
     * 
     * Validations:
     * * id: ID of an existing service, send it as 0 to insert a new entry
     * * owner: Owner account for the PPA, can be updated
     * * name: Name for the PPA, limited to 255 characters, can be updated
     * * biome: Must be one of the following: `pantanal`, `atlanticforest`, `amazonrainforest`, `caatinga`, cannot be updated
     * * location: Make sure location is on the right format. Must be 0.000000-0.000000, cannot be updated
     * * country: Country where the PPA is located, cannot be updated, must be one of the following: `brazil`
     * * ranking: ranking of the PPA, defined by Natuscoin Foundation
     */
    ACTION upsertppa(std::uint64_t id,
                     eosio::name owner,
                     std::string name,
                     std::string biome,
                     std::string location,
                     std::string country,
                     std::string ranking);

    /**
     * Upsert ecoservices
     * 
     * It is an admin only action that insert or update an service. As Natus evolves new ecosystem services will be added
     * Natus Units can have a basket different services with different values that came from the PPA. 
     * 
     * Services are added at each new harvest and cannot be changed after.
     * 
     * Keys are `PPA_id` and `harvest`
     * 
     * Validations:
     * * id: ID of an existing service, send it as 0 to insert a new entry
     * * ppa_id: PPA ID of an existing PPA
     * * harvest_id: ID of an existing harvest
     * * category: must be `water`, `biodiversity` or `carbon`
     * * sub_category: 
     * *    When category is `water`: `course` or `spring`.
     * *    When category is `carbon`: `stock`. 
     * *    When category is `biodiversity`: `vegetation`, `species`, `hotspot`
     * * value: double value corresponding to the service provided
     * 
     * ID   PPA_ID      Harvest     Category     Subcategory     Value
     * 1    gigante1    2020.1      water        spring          10
     * 2    aranhas1    2020.1      water        course          3000
     * 3    gigante1    2021.1      carbon       stock           50
     * 
     */
    ACTION upsertsrv(std::uint64_t id,
                     std::uint64_t ppa_id,
                     eosio::name harvest,
                     std::string category,
                     std::string subcategory,
                     double value);

    /**
     * Setup action, must be the called before `create`, `transfer`, `plant` and `issue`
     * 
     * * natus_symbol: only used when first calling this function, sets the system token
     * * version: always updates the version with the given value
     */
    ACTION setconfig(eosio::symbol natus_symbol, std::string version);

    // TODO: Remove this development only action
    ACTION clean(std::string t, eosio::name scope);

    // Contract configuration singleton
    using configs_type = eosio::singleton<eosio::name{"config"}, config>;
    configs_type configs_singleton;

    typedef eosio::multi_index<eosio::name{"accounts"}, accounts> accounts_table;
    typedef eosio::multi_index<eosio::name{"units"}, units> units_table;
    typedef eosio::multi_index<eosio::name{"ecoservices"}, ecoservices> ecoservices_table;
    typedef eosio::multi_index<eosio::name{"harvests"}, harvests> harvest_table;
    typedef eosio::multi_index<eosio::name{"ppas"}, ppas> ppa_table;

private:
    void _checkconfig();
    eosio::time_point_sec _now();
    uint64_t get_available_id(std::string table);

    // Saves new entries to the Units table
    void _mint(eosio::name to, eosio::name issuer, eosio::name harvest, std::uint64_t ppa_id, std::string report_hash);

    // Add / Subtract balances on the Accounts table
    void _add_balance(eosio::name owner, eosio::asset quantity, std::uint64_t ppa_id, eosio::name harvest);
    void _sub_balance(eosio::name owner, eosio::asset quantity, std::uint64_t ppa_id, eosio::name harvest);
};