.PHONY: natus.wasm

src = $(wildcard * .cpp)
obj = $(src:.cpp=.wasm)

natus.wasm: $(src)
	eosio-cpp -o $@ $^ -abigen -R ./ricardian

clean:
	rm $(obj)