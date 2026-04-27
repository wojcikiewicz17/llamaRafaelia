.PHONY: all clean run-x86 export-rmrcti

all: export-rmrcti
	./build-asm.sh

run-x86: all
	./dist/hello_x86_64

export-rmrcti:
	./export-rmrcti-asm.sh

clean:
	rm -rf dist rmrCti_x86_64
