.PHONY: natus.wasm

src = $(wildcard *.cpp)
obj = $(src:.cpp=.wasm)
version = v1.0
commithash = $(shell git log -1 --pretty=format:"%h")

url = https://staging.cambiatus.io
contract = natusunitdev
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
	cleos -u $(url) push action $(contract) clean '["units", ""]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["ecoservices", ""]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["harvest", ""]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["ppa", ""]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["indexes", ""]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["accounts", "lucca"]' -p $(authorization)

fill:
	# PPAs
	cleos -u $(url) push action $(contract) upsertppa '[0, "$(ppa1)", "RPPN Orion", "atlanticforest", "-20.378172,-43.416413", "brazil", "A"]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertppa '[0, "$(ppa1)", "RPPN Atlas", "amazonrainforest", "-88.378172,9.416413", "brazil", "B"]' -p $(authorization)

	# Harvests
	cleos -u $(url) push action $(contract) sow '["2k21.1", "$(issuer)", 0, 0, "1000 NSTU", 364, "s3.aws.com/bucket/2k21.1/"]' -p $(authorization)
	cleos -u $(url) push action $(contract) sow '["2k21.2", "$(issuer)", 0, 1, "200 NSTU", 30, "s3.aws.com/bucket/2k21.2/"]' -p $(authorization)

	# Ecoservices
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "2k21.1", "biodiversity", "vegetation", 100.8]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "2k21.1", "water", "spring", 3.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "2k21.1", "water", "course", 15.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "2k21.1", "biodiversity", "species", 5.0]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertsrv '[0, 1, "2k21.1", "carbon", "stock", 18054054.88]' -p $(authorization)

issue: 
	cleos -u $(url) push action $(contract) issue '["lucca", 1, "2k21.1", "10 NSTU", "somehash", "First time issuing Natus"]' -p $(issuer)
	# cleos -u $(url) push action $(contract) issue '["karla", 1, "2k21.1", "10 NSTU", "somehash", "First time issuing Natus"]' -p $(authorization)