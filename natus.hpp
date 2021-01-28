#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

class [[eosio::contract("natus")]] natus : public eosio::contract
{
public:
    using contract::contract;

    /**
     * Table that holds information about all Natus Units
     * Natus Unit is a digital good that represents a basket of services provided by a given RPPN (see links below) in a given period called `harvest`
     * 
     * What can you do with it?
     * Like any digital good Natus Units can be exchanged between users.
     * You can also choose to plant it. It will be in cultivation phase and will yield the planter a reward in the next harvest.
     * Planted Natus U are cannot be exchanged anymore and represent a commitment with the land and nature represented by the Natus Unit
     * 
     * Here is a general description of available fields on a Natus Unit:
     * 
     * * id: ID
     * * owner: Current owner 
     * * origin: RPPN ID
     * * harvest: Period of harvest, usually happens on 365 days cycle but not always
     * * report_hash: Hash for the full report document, used to garantee authenticity of the document
     * * biome: RPPN's biome
     * * location: RPPN's location using latitude and longitude
     * * planted_date: Date it was planted, its empty if it is still not planted
     * 
     * https://en.wikipedia.org/wiki/Private_natural_heritage_reserve_(Brazil)
     * https://pt.wikipedia.org/wiki/Reserva_particular_do_patrimônio_natural
     */
    TABLE units
    {
        std::uint32_t id;
        eosio::name owner;
        std::string origin;
        // std::uint64_t harvest;
        std::string harvest;
        std::string report_hash;
        std::string biome;
        std::string location;

        std::uint32_t planted_date;

        std::uint32_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(units, (id)(owner)(origin)(harvest)(report_hash)(biome)(location)(planted_date));
    };

    // TODO: How should we include that? On issue directly?
    TABLE services
    {
        std::uint32_t id;
        std::uint32_t unit_id;
        std::string category;
        std::string subcategory;
        std::float value;

        std::uint32_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(services, (id)(unit_id)(category)(subcategory)(value));
    };

    TABLE harvest{};

    TABLE rppn
    {
        std::uint32_t id;
        eosio::name owner;
        std::string name;
        std::string biome;
        std::string location;

        std::uint32_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(rppn, (id)(owner)(name)(location)(biome));
    };

    /**
     * Issue a new Natus Unit
     * 
     * Validations:
     * * Owner: Owner EOSIO Account, must be valid and exist
     * * Origin: Origin RPPN ID, must be present in the RPPN table
     * * Harvest: Harvest ID, must be present on the Havest table
     * * Report Hash: must be 256 chars long using MD-5
     * * Biome: Must be one of the following: `pantanal`, `atlanticflorest`, `amazonrainflorest`
     * * Location: Make sure location is on the right format. Must be 0.000000-0.000000
     */
    ACTION issue(eosio::name to, eosio::name owner, std::string origin, std::string harvest, std::string report_hash std::string biome, std::string location);

    /**
     * Plant a Natus Unit
     * 
     * Validations:
     * * ID: ID of the Natus Unit, must exist and not be planted yet
     * * Owner: Owner EOSIO Account, must be valid and exist. Also should be the owner of the given Natus Unit ID
     * * Date: Must be always on the format `dd/mm/aaaa`
     */
    ACTION plant(std::uint32_t id, eosio::name owner, std::string date);

    /**
     * Upsert RPPN 
     * Admin only action that insert or update an RPPN
     * 
     * Validations:
     * * Id: ID of an existing service, send it as 0 to insert a new entry
     * * Owner: Owner account for the RPPN
     * * Name: Name for the RPPN, limited to 255 characters
     * * Biome: Must be one of the following: `pantanal`, `atlanticflorest`, `amazonrainflorest`
     * * Location: Make sure location is on the right format. Must be 0.000000-0.000000
     */
    ACTION upsertrppn(std::uint32_t id, eosio::name owner, std::string name, std::string biome, std::string location);

    /**
     * Upser services
     * 
     * It is an admin only action that insert or update an service. As Natus evolves new ecosystem services will be added
     * Natus Units can have a basket different services with different values. 
     * 
     * Validations:
     * * Id: ID of an existing service, send it as 0 to insert a new entry
     * * Unit ID: Natus Unit ID. Must exist
     * * Category: must be `water`, `biodiversity` or `carbon`
     * * Sub category: 
     * *    When category is `water`: `course` or `sprint`.
     * *    When category is `carbon`: `capture`. 
     * *    When category is `biodiversity`: `vegetation`, `species`, `hotspot`
     * * Value: float value corresponding to the service provided
     */
    // TODO: Should we allow update on a Natus Unit services? does it makes sense?
    ACTION upsertservices(std::uint32_t id, std::uint32_t unit_id, std::string category, std::string subcategory, std::float value);
}