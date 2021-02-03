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

fill:
	cleos -u $(url) push action $(contract) upsertppa '[0, "$(ppa1)", "RPPN Orion", "atlanticflorest", "-20.378172,-43.416413", "brazil", "A"]' -p $(authorization)