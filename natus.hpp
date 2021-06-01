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
     * * last_ppa_id: system's last used ppa_id
     * * last_ecoservice_id: system's last used ecoservice_id
     */
    TABLE config
    {
        eosio::symbol natus_symbol;       // Symbol used to represent Natus Units
        std::string version;              // Current version for the contract
        eosio::asset total_issued_supply; // Total supply of Natus Unit issued, counts planted ones

        // Custom indexing
        std::uint64_t last_ppa_id;        // Last used ppa_id
        std::uint64_t last_ecoservice_id; // Last used ecoservice_id
    };

    /**
     * Store the reports for a collection and ppa pair. It can only hold one collection/ppa per report
    */
    TABLE reports
    {
        // Scope is self
        std::uint64_t id;
        eosio::name collection;
        std::uint64_t ppa_id;
        std::optional<std::string> report_hash;

        std::uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(reports, (id)(collection)(ppa_id)(report_hash));
    };

    /**
     * Table that holds an account balance
     * Natus Units always are created based on a single PPA resourses as a way to represent its ecosystem services.
     * For that reason we use both Collection and PPA to indentify the origin of the 
     * 
     * So a single user can have multiple Natus Units balance, each one representing the portion from each PPA/Collection
    */
    TABLE accounts
    {
        // Scope is owner
        std::uint64_t ppa_collection_id; // Key calculated from merging ppa_id and collection
        eosio::asset balance;            // Amount of units
        std::uint64_t ppa_id;            // PPA that created the Unit
        eosio::name collection;          // Collections where those amounted units comes from

        std::uint64_t primary_key() const { return ppa_collection_id; }

        EOSLIB_SERIALIZE(accounts, (ppa_collection_id)(balance)(ppa_id)(collection));
    };

    /**
     * Collection is grouping of Natus Units issued into the system.
     * 
     * This table holds configurations and characteristics of the Collection. Each collection is handled by an `issuer`
     * 
     * * id: incremental ID of the collection
     * * name: name, used as foreign_key and Units scope
     * * sellable: if the units of this collection can be selled by Natus Foundation. This value can change over time
     * * transferable: if the units can be transferred between users, after a Collection ends, its no longer possible
     * * max_supply: max possible supply
     * * current_supply: supply in circulation, don't include planted units
     * * issued_supply: supply issued so far, always smaller than max_supply but can be higher than current_supply
     * * issue_window: max date where new units can be issued, after that date it will not be possible to issue new ones
     * * base_uri: base URI for storing the Units certificates
     * * created_at: creation date for the Collection
     */
    TABLE collection
    {
        // Scope is _self
        eosio::name name;                       // Eg.: "2k21.1", "2k21.2"
        eosio::name issuer;                     // Issuer account
        bool sellable;                          // Informs if its sellable
        bool transferable;                      // Informs if its transferable
        eosio::asset max_supply;                // Whole collection assets
        eosio::asset current_supply;            // Supply in circulation, don't include planted units
        eosio::asset issued_supply;             // Everything issued so far
        eosio::time_point_sec available_window; // Max available date (Transfer, Plant or Issue)
        std::string base_uri;                   // Base URI for the Collection Units
        eosio::time_point_sec created_at;       // Date of creation

        std::uint64_t primary_key() const { return name.value; }

        EOSLIB_SERIALIZE(collection, (name)(issuer)(sellable)(transferable)(max_supply)(current_supply)(issued_supply)(available_window)(base_uri)(created_at));
    };

    /**
     * Eco services provided by a PPA during a Collection.
     * It allows for certain services to be informed on a PPA provided services on a given Collection
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
        eosio::name collection;
        std::string category;
        std::string subcategory;
        double value;

        std::uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(ecoservices, (id)(ppa_id)(collection)(category)(subcategory)(value));
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
     * Create a new collection or update an existing one. 
     * Collection usually happen anually but can happen other times in special cases.
     * 
     * Update only changes the following properties: `sellable`, `transferable`, `issue_days`, `base_uri`
     * 
     * Validations:
     * * id: used to update an existing collection, use 0 to create a new one
     * * name: name of the collection
     * * issuer: issuer account
     * * sellable: informs if this collection units are sellable from Natus Foundation
     * * transferable: informs if its possible to transfer Units
     * * max_supply: maximum supply possible on this collection
     * * issue_days: # of days where the issue will be available 
     * * base_uri: base URI for all certificates for units on this collection
     */
    ACTION upsertcollec(eosio::name name,
                        eosio::name issuer,
                        bool sellable,
                        bool transferable,
                        eosio::asset max_supply,
                        std::uint32_t issue_days,
                        std::string base_uri);

    /**
     * Saves a report for a given Collection and PPA. Must be unique
     * 
     * Validations:
     * * collection: must exist, combination with ppa must be unique
     * * ppa_id: id for the ppa, must exist, combination with collection must be unique
     * * report_hash: hash for the report that attest for authenticity of the issued NSTU
     */
    ACTION upsertreport(eosio::name collection, std::uint64_t ppa_id, std::string report_hash);

    /**
     * Issue a new Natus Unit
     * 
     * * to: destination EOSIO account, must be valid and exist
     * * ppa_id: origin PPA ID, must be present in the PPA table
     * * collection: collection name, must be present on the Havest table
     * * quantity: amount that need to be issued
     * * report_hash: hash of the report corresponding to this PPA's Natus Units
     */
    ACTION issue(eosio::name to,
                 std::uint64_t ppa_id,
                 eosio::name collection,
                 eosio::asset quantity,
                 std::string memo);

    /**
     * Plant a Natus Unit
     * 
     * Validations:
     * * PPA ID: ID of the PPA for the Natus Units that will be planted
     * * Collection: Collection name from where the planted Natus belong
     * * Quantity: amount of NSTU to be planted
     * * Owner: Owner EOSIO Account, must be valid and exist. Also should be the owner of the given Natus Unit ID
     * * Memo: Optional field for memo
     */
    ACTION plant(std::uint64_t ppa_id,
                 eosio::name collection_name,
                 eosio::asset quantity,
                 eosio::name owner,
                 std::string memo);

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
                    std::uint64_t ppa_id,
                    eosio::name collection,
                    eosio::asset quantity,
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
     * Services are added at each new collection and cannot be changed after.
     * 
     * Keys are `PPA_id` and `collection`
     * 
     * Validations:
     * * id: ID of an existing service, send it as 0 to insert a new entry
     * * ppa_id: PPA ID of an existing PPA
     * * collection_id: ID of an existing collection
     * * category: must be `water`, `biodiversity` or `carbon`
     * * sub_category: 
     * *    When category is `water`: `course` or `spring`.
     * *    When category is `carbon`: `stock`. 
     * *    When category is `biodiversity`: `vegetation`, `species`, `hotspot`
     * * value: double value corresponding to the service provided
     * 
     * ID   PPA_ID      Collection     Category     Subcategory     Value
     * 1    gigante1    2020.1      water        spring          10
     * 2    aranhas1    2020.1      water        course          3000
     * 3    gigante1    2021.1      carbon       stock           50
     * 
     */
    ACTION upsertsrv(std::uint64_t id,
                     std::uint64_t ppa_id,
                     eosio::name collection,
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

    ACTION clean(std::string t, eosio::name scope);

    // Contract configuration singleton
    using configs_type = eosio::singleton<eosio::name{"config"}, config>;
    configs_type configs_singleton;

    typedef eosio::multi_index<eosio::name{"accounts"}, accounts> accounts_table;
    typedef eosio::multi_index<eosio::name{"ecoservices"}, ecoservices> ecoservices_table;
    typedef eosio::multi_index<eosio::name{"collection"}, collection> collection_table;
    typedef eosio::multi_index<eosio::name{"reports"}, reports> reports_table;
    typedef eosio::multi_index<eosio::name{"ppas"}, ppas> ppa_table;

private:
    void _checkconfig();
    eosio::time_point_sec _now();
    uint64_t get_available_id(std::string table);

    // Saves new entries to the Units table
    void _mint(eosio::name to, eosio::name issuer, eosio::name collection, std::uint64_t ppa_id, std::string report_hash);

    // Add / Subtract balances on the Accounts table
    void _add_balance(eosio::name owner, eosio::asset quantity, std::uint64_t ppa_id, eosio::name collection);
    void _sub_balance(eosio::name owner, eosio::asset quantity, std::uint64_t ppa_id, eosio::name collection);
};
