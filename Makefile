# Перша ціль = default. `make` без аргументів покаже help.
.PHONY: help build format lint test quality clean

help:
	@echo "Available targets:"
	@echo "  make build    — cmake configure + build (debug preset)"
	@echo "  make format   — clang-format -i for all C++ files"
	@echo "  make lint     — clang-tidy for all C++ files"
	@echo "  make test     — ctest for all unit-tests"
	@echo "  make quality  — format + lint + test (run before PR)"
	@echo "  make clean    — remove build folder"

build:
	cmake --preset debug
	cmake --build --preset debug

test: build
	ctest --test-dir build/debug --output-on-failure

quality: format lint test

clean:
	rm -rf build

# Каталог модуля курсової. Одне місце правди — решта цілей беруть звідси.
MODULE := telemetry

format:
	find $(MODULE) -type f \( -name '*.cpp' -o -name '*.hpp' \) -exec clang-format -i {} +
	cmake-format -i $(MODULE)/CMakeLists.txt

lint: build
	clang-tidy -p build/debug $(MODULE)/src/*.cpp $(MODULE)/tests/*.cpp