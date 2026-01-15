EXTRA_DIST = testsuite.sh builtins command lexer parser pipe pretty_printer redirection variable_expansion

check-local:
	@echo "Running testsuite..."
	@BIN_PATH="$(BIN_PATH)" COVERAGE="$(COVERAGE)" OUTPUT_FILE="$(OUTPUT_FILE)" ./testsuite.sh || true

TESTS = testsuite.sh \
        builtins/simple/echo.sh

TESTS_ENVIRONMENT = BIN_PATH=$(BIN_PATH) COVERAGE=$(COVERAGE) OUTPUT_FILE=$(OUTPUT_FILE)

check: check-local