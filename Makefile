# Перша ціль = default. `make` без аргументів покаже help.
.PHONY: help build format lint test demo quality clean

help:
	@echo "Available targets:"
	@echo "  make build    — cmake configure + build (debug preset)"
	@echo "  make format   — clang-format -i for all C++ files"
	@echo "  make lint     — clang-tidy for all C++ files"
	@echo "  make test     — ctest for all unit-tests"
	@echo "  make demo     — build + run experiment & rms, write CSV/plots to docs/"
	@echo "  make quality  — format + lint + test (run before PR)"
	@echo "  make clean    — remove build folder"

build:
	cmake --preset debug
	cmake --build --preset debug

test: build
	ctest --test-dir build/debug --output-on-failure

# Демонстрація: обидва прогони -> CSV у docs/, і графіки (якщо є matplotlib).
demo: build
	@mkdir -p docs
	@echo "== Якісний прогін: траєкторія по колу, блекаут 8-10с, коридор +-3sigma =="
	./build/debug/$(MODULE)/experiment > docs/experiment.csv
	@echo "== Кількісний прогін: RMS помилки vs рівень втрат =="
	./build/debug/$(MODULE)/rms > docs/rms.csv
	@echo "== Графіки (потрібен python3 + matplotlib) =="
	@python3 scripts/plot_experiment.py docs/experiment.csv || echo "  matplotlib недоступний -> дивись docs/experiment.csv"
	@python3 scripts/plot_rms.py docs/rms.csv || echo "  matplotlib недоступний -> дивись docs/rms.csv"
	./build/debug/$(MODULE)/realtime > docs/realtime.csv
	@python3 scripts/plot_realtime.py docs/realtime.csv || echo "  matplotlib недоступний -> дивись docs/realtime.csv"
	@echo "Готово. Результати у docs/ (experiment.csv/.png, rms.csv/.png)."

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