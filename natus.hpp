#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

/**
 * This contract describe the rules and behaviors for Natus Unit. 
 * Natus Unit is a digital good that represents the services provided by nature within a PPA (Private Protected Area)
 * 
 * This is an effort to bring funds to those areas to help it grow an be preserved.
 * With the funds obtained from the Natus Unit project, PPAs can study their land, discover what ecossystem services nature provide.
 * That data allow measure of the production of those ecossystem services.
 * Each year Natus Foundation measure that production and generate Natus Units, that comes from the PPAs, that measure is called `harvest`
 * 
 * # What can you do with it?
 * 
 * Like any digital good Natus Units can be exchanged between users.
 * You can also choose to plant it. It will be in cultivation phase and will yield the planter a reward in the next harvest.
 * Planted Natus U are cannot be exchanged anymore and represent a commitment with the land and nature represented by the Natus Unit
 * 
 * ### Links
 * 
 * https://en.wikipedia.org/wiki/Private_protected_area
 * 
 * #### Implementation of PPA in Brazil
 * 
 * https://en.wikipedia.org/wiki/Private_natural_heritage_reserve_(Brazil)
 * https://pt.wikipedia.org/wiki/Reserva_particular_do_patrimônio_natural
 */
class [[eosio::contract("natus")]] natus : public eosio::contract
{
public:
    using contract::contract;

    /**
     * Table that holds information about all Natus Units
     * Natus Unit is a digital good that represents a basket of services provided by a given PPA (see links below) in a given period called `harvest`
     * 
     * Here is a general description of available fields on a Natus Unit:
     * 
     * * id: ID
     * * owner: Current owner 
     * * origin: PPA ID
     * * harvest: Period of harvest, usually happens on 365 days cycle but not always
     * * report_hash: Hash for the full report document, used to garantee authenticity of the document
     * * planted_at: Date it was planted, its empty if it is still not planted
     * * inserted_at: Date it was inserted/created
     * * updated_at: Date it was updated
     */
    TABLE units
    {
        std::uint64_t id;
        eosio::name owner;
        std::string origin;
        std::string harvest;
        std::string report_hash;

        std::uint64_t planted_at;
        std::uint64_t inserted_at;
        std::uint64_t updated_at;

        std::uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(units, (id)(owner)(origin)(harvest)(report_hash)(planted_at)(inserted_at)(updated_at));
    };

    /**
     * Eco services provided by a PPA during a Harvest.
     * It allows for certain services to be informed on a PPA provided services on a given Harvest
     * 
     * Available `category`s: `water`, `carbon` and `biodiversity`
     * Available `subcategory`s: `course`, `spring`, `stock`, `vegetation`, `species`, `hotspot`
     * Value is a float, can be interpreted as tons, numbers, m2, etc
     */
    TABLE ecoservices
    {
        std::uint64_t id;
        std::uint64_t ppa_id;
        std::uint64_t harvest_id;
        std::string category;
        std::string subcategory;
        float value;

        std::uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(ecoservices, (id)(ppa_id)(harvest_id)(category)(subcategory)(value));
    };

    /**
     * Harvest is the moment where an PPA generates new Natus Unit to the system
     * 
     * It involves also considering current investors, investors from the last harvest, etc
     */
    TABLE harvest
    {
        std::uint64_t id;
        std::uint64_t year;
        std::string name; // Eg.: "2021.1", "2021.2"

        std::uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(harvest, (id)(year)(name));
    };

    /**
     * Central data structure, represents a PPA
     * 
     * PPAs can exist in different names depending on the country, with different legislations with the same country
     * We rank the PPA based on a criterea on how good they are on preserve the nature, it also affects the initial price offering
     * 
     * PPAs can be updated to change its name, owner and ranking
    */
    TABLE ppa
    {
        std::uint64_t id;
        eosio::name owner;
        std::string name;
        std::string biome;
        std::string location;
        std::string country;
        std::string ranking;

        std::uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(ppa, (id)(owner)(name)(biome)(location)(country)(ranking));
    };

    /**
     * Issue a new Natus Unit
     * 
     * Validations:
     * * Owner: Owner EOSIO Account, must be valid and exist
     * * PPA ID: Origin PPA ID, must be present in the PPA table
     * * Harvest: Harvest ID, must be present on the Havest table
     * * Report Hash: must be 256 chars long using MD-5
     */
    ACTION issue(eosio::name to, eosio::name owner, std::uint64_t ppa_id,
                 std::uint64_t harvest_id, std::string report_hash);

    /**
     * Plant a Natus Unit
     * 
     * Validations:
     * * ID: ID of the Natus Unit, must exist and not be planted yet
     * * Owner: Owner EOSIO Account, must be valid and exist. Also should be the owner of the given Natus Unit ID
     */
    ACTION plant(std::uint64_t id, eosio::name owner);

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
    ACTION transfer(eosio::name from, eosio::name to, std::uint64_t unit_id, std::string memo);

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
    ACTION upsertppa(std::uint64_t id, eosio::name owner, std::string name, std::string biome,
                     std::string location, std::string country, std::string ranking);

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
     * * value: float value corresponding to the service provided
     * 
     * ID   PPA_ID      Harvest     Category     Subcategory     Value
     * 1    gigante1    2020.1      water        spring          10
     * 2    aranhas1    2020.1      water        course          3000
     * 3    gigante1    2021.1      carbon       stock           50
     * 
     */
    ACTION upsertsrv(std::uint64_t id, std::uint64_t ppa_id, std::uint64_t harvest_id,
                     std::string category, std::string subcategory, float value);

    /**
     * Adds a Harvest to the system. Usually they happen anually but can happen other times in special cases.
     * 
     * Validations:
     * - ID: Used to update an existing harvest, send 0 to create a new one
     * - Year: Year of the harvest, must be at least 2021
     * - Name: Name of the harvest, must be less than 255 characters
     */
    ACTION upserthrvst(std::uint64_t id, std::uint64_t year, std::string name);

    // TODO: Remove this development only action
    ACTION clean(std::string t);

    typedef eosio::multi_index<eosio::name{"units"}, units> units_table;
    typedef eosio::multi_index<eosio::name{"ecoservices"}, ecoservices> ecoservices_table;
    typedef eosio::multi_index<eosio::name{"harvest"}, harvest> harvest_table;
    typedef eosio::multi_index<eosio::name{"ppa"}, ppa> ppa_table;
};