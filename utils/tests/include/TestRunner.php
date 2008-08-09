<?php

class TestRunner {
	function __construct()
	{
	}

	public function run(Build $build)
	{
		$binary_name = $build->getTestName();
		if ($binary_name === false)
		{
			return;
		}
		$test_output = '<UnitTest>'.shell_exec("./$binary_name --log_format=XML --report_format=XML 2>&1").'</UnitTest>';

		//$test_output = '<UnitTest><TestLog><Message file="./boost/test/impl/results_collector.ipp" line="220">Test case test_user_team_name doesn&apos;t include any assertions</Message><Error file="build/debug/tests/test_config_cache.cpp" line="84">check defines_map.size() == cache.get_preproc_map().size() failed [2 != 0]</Error></TestLog>
		//<TestResult><TestSuite name="wesnoth unit tests master suite" 
		//result="failed" 
		//assertions_passed="112" 
		//assertions_failed="1" 
		//expected_failures="0" 
		//test_cases_passed="17" 
		//test_cases_failed="1" 
		//test_cases_skipped="0" 
		//test_cases_aborted="0"></TestSuite>
		//</TestResult></UnitTest>';
		$xml = simplexml_load_string($test_output);
		if (!($xml instanceof SimpleXMLElement)
			|| !isset($xml->TestLog[0]) )
		{
			global $db;
			echo $test_output;
			$db->FailTrans();
			return;
		}
		foreach($xml->TestLog[0] as $name => $data)
		{
			$test_error = new TestError($name, $data, $build);
			$test_error->updateDB($build);
		}

		foreach($xml->TestResult[0] as $name => $data)
		{
			$test_report = new TestResult($data, $build);
			$test_report->updateDB();
		}
	}
}

?>
