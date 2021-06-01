.PHONY: natus.wasm

src = $(wildcard *.cpp)
obj = $(src:.cpp=.wasm)
version = v1.0
commithash = $(shell git log -1 --pretty=format:"%h")

url = https://staging.cambiatus.io
contract = natusunitdev
# contract = natusunitd3v
authorization = $(contract)@active
ppa1 =	natusppadev1
issuer = natusfoundat
collection = araraazul

natus.wasm: $(src)
	eosio-cpp -o $@ $^ -abigen -R ./ricardian

clean:
	rm $(obj)

deploy: 
	make
	cleos -u $(url) set contract $(contract) ../natus --use-old-rpc
	cleos -u $(url) push action $(contract) setconfig '["0,NSTU", "$(version)-$(commithash)"]' -p $(authorization)
	cleos -u $(url) set account permission --add-code $(contract) active

erase:
	cleos -u $(url) push action $(contract) clean '["ecoservices", ""]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["collection", ""]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["ppa", ""]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["indexes", ""]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["accounts", "lucca"]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["accounts", "karla"]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["accounts", "$(issuer)"]' -p $(authorization)

fill:
	# PPAs
	cleos -u $(url) push action $(contract) upsertppa '[0, "$(ppa1)", "RPPN Sítio Bons Amigos", "amazonrainforest", "-2.834543,-60.076562", "brazil", "A"]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertppa '[0, "$(ppa1)", "RPPN Gigante do Itaguaré", "atlanticforest", "-22.497423,-45.092014", "brazil", "A"]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertppa '[0, "$(ppa1)", "RPPN La Figueira", "atlanticforest", "-23.032616,-46.08640", "brazil", "A"]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertppa '[0, "$(ppa1)", "RPPN Morro das Aranhas", "atlanticforest", "-27.468653,-48.381393", "brazil", "A"]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertppa '[0, "$(ppa1)", "RPPN Neivo Pires", "pantanal", "-20.197960,56.47900", "brazil", "A"]' -p $(authorization)

	# Collection
	cleos -u $(url) push action $(contract) upsertcollec '["araraazul", "$(issuer)", 0, 1, "9719479 NSTU", 364, "https://natus.s3-sa-east-1.amazonaws.com/2020/"]' -p $(authorization)

	# Ecoservices - Sítio Bons Amigos
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "$(collection)", "water", "course", 193]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "$(collection)", "biodiversity", "vegetation", 31.92]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "$(collection)", "biodiversity", "species", 14.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "$(collection)", "carbon", "stock", 8452.30]' -p $(authorization)

	# Ecoservices - Gigante do Itaguaré
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "$(collection)", "water", "spring", 31.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "$(collection)", "water", "course", 18018.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "$(collection)", "biodiversity", "vegetation", 356.89]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "$(collection)", "biodiversity", "hotspot", 1.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "$(collection)", "biodiversity", "species", 32.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "$(collection)", "carbon", "stock", 135190.15]' -p $(authorization)

	# Ecoservices - La Figueira
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "$(collection)", "water", "spring", 4.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "$(collection)", "water", "course", 1110.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "$(collection)", "biodiversity", "vegetation", 33.53]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "$(collection)", "biodiversity", "hotspot", 1.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "$(collection)", "biodiversity", "species", 23.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "$(collection)", "carbon", "stock", 9614.22]' -p $(authorization)

	# Ecoservices - Morro das Aranhas
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "$(collection)", "water", "spring", 12.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "$(collection)", "water", "course", 3196.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "$(collection)", "biodiversity", "vegetation", 43.63]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "$(collection)", "biodiversity", "hotspot", 1.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "$(collection)", "biodiversity", "species", 6.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "$(collection)", "carbon", "stock", 9967.88]' -p $(authorization)

	# Ecoservices - Neivo Pires
	cleos -u $(url) push action $(contract) upsertsrv '[0, 5, "$(collection)", "water", "course", 6220]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 5, "$(collection)", "biodiversity", "vegetation", 461.91]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 5, "$(collection)", "biodiversity", "species", 21.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 5, "$(collection)", "carbon", "stock", 147104.91]' -p $(authorization)

issue: 
	# Sítio bons amigos
	cleos -u $(url) push action $(contract) issue '["natusfoundat", 1, "$(collection)", "319748 NSTU", "Natus Staging tests"]' -p $(issuer)
	cleos -u $(url) push action $(contract) upsertreport '["$(collection)", 1, "bons-amigos-8e73e5dfff77d11a295595a2ecb4fc13.pdf"]' -p $(authorization)

	# Gigante do Itaguaré
	cleos -u $(url) push action $(contract) issue '["natusfoundat", 2, "$(collection)", "3586449 NSTU", "Natus Staging tests"]' -p $(issuer)
	cleos -u $(url) push action $(contract) upsertreport '["$(collection)", 2, "gigante-do-itaguare-e12cf34343950716d2418dea50689e83.pdf"]' -p $(authorization)

	# La Figueira
	cleos -u $(url) push action $(contract) issue '["natusfoundat", 3, "$(collection)", "336782 NSTU", "Natus Staging tests"]' -p $(issuer)
	cleos -u $(url) push action $(contract) upsertreport '["$(collection)", 3, "la-figueira-c0e627fc809fe81dacb268e3f0415b7b.pdf"]' -p $(authorization)

	# Morro das Aranhas
	cleos -u $(url) push action $(contract) issue '["natusfoundat", 4, "$(collection)", "441600 NSTU", "Natus Staging tests"]' -p $(issuer)
	cleos -u $(url) push action $(contract) upsertreport '["$(collection)", 4, "morro-das-aranhas-4a9d81d9366cc30019b9401dd49ead0e.pdf"]' -p $(authorization)

	# Neivo Pires
	cleos -u $(url) push action $(contract) issue '["natusfoundat", 5, "$(collection)", "5034900 NSTU", "Natus Staging tests"]' -p $(issuer) 
	cleos -u $(url) push action $(contract) upsertreport '["$(collection)", 5, "neivo-pires-b06aff744564bb7cb0c8bc4dc26f1b96.pdf"]' -p $(authorization)
