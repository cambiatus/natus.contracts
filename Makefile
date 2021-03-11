.PHONY: natus.wasm

src = $(wildcard *.cpp)
obj = $(src:.cpp=.wasm)
version = v1.0
commithash = $(shell git log -1 --pretty=format:"%h")

url = https://staging.cambiatus.io
# contract = natusunitdev
contract = natusunitd3v
authorization = $(contract)@active
ppa1 =	natusppadev1
issuer = natusfoundat

natus.wasm: $(src)
	eosio-cpp -o $@ $^ -abigen -R ./ricardian

clean:
	rm $(obj)

deploy: 
	make
	cleos -u $(url) set contract $(contract) ../natus --use-old-rpc
	cleos -u $(url) push action $(contract) setconfig '["0,NSTU", "$(version)-$(commithash)"]' -p $(authorization)

erase:
	cleos -u $(url) push action $(contract) clean '["ecoservices", ""]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["harvest", ""]' -p $(authorization)
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

	# Harvests
	cleos -u $(url) push action $(contract) sow '["1", "$(issuer)", 0, 0, "9719479 NSTU", 364, "https://natus.s3-sa-east-1.amazonaws.com/2020/"]' -p $(authorization)

	# Ecoservices - Sítio Bons Amigos
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "1", "water", "course", 193]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "1", "biodiversity", "vegetation", 31.92]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "1", "biodiversity", "species", 14.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "1", "carbon", "stock", 8452.30]' -p $(authorization)

	# Ecoservices - Gigante do Itaguaré
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "1", "water", "spring", 31.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "1", "water", "course", 18018.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "1", "biodiversity", "vegetation", 356.89]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "1", "biodiversity", "hotspot", 1.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "1", "biodiversity", "species", 32.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 2, "1", "carbon", "stock", 135190.15]' -p $(authorization)

	# Ecoservices - La Figueira
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "1", "water", "spring", 4.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "1", "water", "course", 1110.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "1", "biodiversity", "vegetation", 33.53]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "1", "biodiversity", "hotspot", 1.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "1", "biodiversity", "species", 23.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 3, "1", "carbon", "stock", 9614.22]' -p $(authorization)

	# Ecoservices - Morro das Aranhas
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "1", "water", "spring", 12.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "1", "water", "course", 3196.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "1", "biodiversity", "vegetation", 43.63]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "1", "biodiversity", "hotspot", 1.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "1", "biodiversity", "species", 6.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 4, "1", "carbon", "stock", 9967.88]' -p $(authorization)

	# Ecoservices - Neivo Pires
	cleos -u $(url) push action $(contract) upsertsrv '[0, 5, "1", "water", "course", 6220]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 5, "1", "biodiversity", "vegetation", 461.91]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 5, "1", "biodiversity", "species", 21.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 5, "1", "carbon", "stock", 147104.91]' -p $(authorization)

issue: 
	# Sítio bons amigos
	cleos -u $(url) push action $(contract) issue '["natusfoundat", 1, "1", "319748 NSTU", "Natus Staging tests"]' -p $(issuer)
	cleos -u $(url) push action $(contract) upsertreport '["1", 1, "bons-amigos-8e73e5dfff77d11a295595a2ecb4fc13.pdf"]' -p $(authorization)

	# Gigante do Itaguaré
	cleos -u $(url) push action $(contract) issue '["natusfoundat", 2, "1", "3586449 NSTU", "Natus Staging tests"]' -p $(issuer)
	cleos -u $(url) push action $(contract) upsertreport '["1", 2, "gigante-do-itaguare-e12cf34343950716d2418dea50689e83.pdf"]' -p $(authorization)

	# La Figueira
	cleos -u $(url) push action $(contract) issue '["natusfoundat", 3, "1", "336782 NSTU", "Natus Staging tests"]' -p $(issuer)
	cleos -u $(url) push action $(contract) upsertreport '["1", 3, "la-figueira-c0e627fc809fe81dacb268e3f0415b7b.pdf"]' -p $(authorization)

	# Morro das Aranhas
	cleos -u $(url) push action $(contract) issue '["natusfoundat", 4, "1", "441600 NSTU", "Natus Staging tests"]' -p $(issuer)
	cleos -u $(url) push action $(contract) upsertreport '["1", 4, "morro-das-aranhas-4a9d81d9366cc30019b9401dd49ead0e.pdf"]' -p $(authorization)

	# Neivo Pires
	cleos -u $(url) push action $(contract) issue '["natusfoundat", 5, "1", "5034900 NSTU", "Natus Staging tests"]' -p $(issuer) 
	cleos -u $(url) push action $(contract) upsertreport '["1", 5, "neivo-pires-b06aff744564bb7cb0c8bc4dc26f1b96.pdf"]' -p $(authorization)
