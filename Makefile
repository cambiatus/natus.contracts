.PHONY: natus.wasm

src = $(wildcard *.cpp)
obj = $(src:.cpp=.wasm)
url = https://staging.cambiatus.io
contract = natusunitdev
authorization = $(contract)@active

ppa1 =	natusppadev1

natus.wasm: $(src)
	eosio-cpp -o $@ $^ -abigen -R ./ricardian

clean:
	rm $(obj)

deploy: 
	make
	cleos -u $(url) set contract $(contract) ../natus --use-old-rpc

erase:
	cleos -u $(url) push action $(contract) clean '["ppa"]' -p $(authorization)
	cleos -u $(url) push action $(contract) clean '["harvest"]' -p $(authorization)

fill:
	cleos -u $(url) push action $(contract) upsertppa '[0, "$(ppa1)", "RPPN Orion", "atlanticforest", "-20.378172,-43.416413", "brazil", "A"]' -p $(authorization)
	cleos -u $(url) push action $(contract) upsertppa '[0, "$(ppa1)", "RPPN Atlas", "amazonrainforest", "-88.378172,9.416413", "brazil", "B"]' -p $(authorization)
	cleos -u $(url) push action $(contract) upserthrvst '[0, 2021, "2021.1"]' -p $(authorization)
	cleos -u $(url) push action $(contract) upserthrvst '[0, 2022, "2022.1"]' -p $(authorization)