#include "testMain.h"

CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, TestParser);
	SUITE_ADD_TEST(suite, checkTest);
	SUITE_ADD_TEST(suite, gameOverTest);
	SUITE_ADD_TEST(suite, validMovesTestOld);
	SUITE_ADD_TEST(suite, validMovesTestNew);
	SUITE_ADD_TEST(suite, listIteratorTest);
	SUITE_ADD_TEST(suite, boardToFenTest);
	SUITE_ADD_TEST(suite, setFieldTest);
	SUITE_ADD_TEST(suite, getFieldTest);
	SUITE_ADD_TEST(suite, movePlayerTest);
	SUITE_ADD_TEST(suite, minimaxTreeTest);
	SUITE_ADD_TEST(suite, evaluateBoardTest);
	SUITE_ADD_TEST(suite, minimaxTest);

	return suite;
}


int RunAllTests(void)
{
	CuString* output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuite* s2 = CuGetSuite();
	CuSuiteAddSuite(suite, s2);

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
	int failed = suite->failCount;
	CuSuiteDelete(suite);
	CuStringDelete(output);
	return failed;
}

int RunTest(void test(CuTest* tc)) {
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test);

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
	int failed = suite->failCount;

	CuSuiteDelete(suite);
	CuStringDelete(output);

	return failed;
}

int main(int argc, char** argv)
{
	if (argc > 1) {
		char* arg1 = argv[1];

		if (strcmp(arg1, "validMovesTestNew") == 0) {
			return RunTest(validMovesTestNew);
		} else if (strcmp(arg1, "validMovesTestOld") == 0) {
			return RunTest(validMovesTestOld);
		} else if (strcmp(arg1, "gameOverTest") == 0) {
			return RunTest(gameOverTest);
		} else if (strcmp(arg1, "boardToFenTest") == 0) {
			return RunTest(boardToFenTest);
		} else if (strcmp(arg1, "getFieldTest") == 0) {
			return RunTest(getFieldTest);
		} else if (strcmp(arg1, "setFieldTest") == 0) {
			return RunTest(setFieldTest);
		} else if (strcmp(arg1, "movePlayerTest") == 0) {
			return RunTest(movePlayerTest);
		} else if (strcmp(arg1, "fenParserTest") == 0) {
			return RunTest(TestParser);
		} else if (strcmp(arg1, "evaluateBoardTest") == 0) {
			return RunTest(evaluateBoardTest);
		} else if (strcmp(arg1, "minimaxTreeTest") == 0) {
			return RunTest(minimaxTreeTest);
		} else if (strcmp(arg1, "minimaxTest") == 0) {
			return RunTest(minimaxTest);
		} else if (strcmp(arg1, "checkTest") == 0) {
			return RunTest(checkTest);
		} else if (strcmp(arg1, "--help") == 0) {
			printf("Usage: %s [testName]\n", argv[0]);
			printf("If testName is not specified, then all tests are run. testName can be one of the following:\n");
			printf("- validMovesTestNew\n");
			printf("- validMovesTestOld\n");
			printf("- gameOverTest\n");
			printf("- boardToFenTest\n");
			printf("- getFieldTest\n");
			printf("- setFieldTest\n");
			printf("- movePlayerTest\n");
			printf("- fenParserTest\n");
			printf("- evaluateBoardTest\n");
			printf("- minimaxTreeTest\n");
			printf("- minimaxTest\n");
			printf("- checkTest\n");
			exit(0);
		} else {
			printf("Unknown test name: %s\n", arg1);
			exit(1);
		}
	}
	return RunAllTests();
}
