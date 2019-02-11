(* Wolfram Language Test file *)
TestRequirement[$VersionNumber>10.3]
(***************************************************************************************************************************************)
				(*
					Set of Testcases to test tensor apis for library link
				*)
(***************************************************************************************************************************************)
TestExecute[
	currentDirectory = DirectoryName[$CurrentFile];
	Get[FileNameJoin[{ParentDirectory[currentDirectory], "TestConfig.wl"}]];
]

Test[
	Get[FileNameJoin[{currentDirectory, "EchoTensor", "EchoTensor.wl"}]];
	EchoTensor[{1,2,3}]
	,
	{1,2,3}
	,
	TestID->"TensorOperations-20150817-O6E3O4"
]

Test[
	EchoFirst[{1, 2, 3}]
	,
	1
	,
	TestID->"TensorTestSuite-20181121-O5M3H2"
]

Test[
	EchoLast[{1, 2, 3}]
	,
	3
	,
	TestID->"TensorTestSuite-20181121-W3H7Z7"
]

Test[
	r = RandomInteger[1000, {10, 5, 20}];
	EchoElement[NumericArray[r, "Integer64"], {3, 0, 12}]
	,
	r[[4, 1, 13]]
	,
	TestID->"TensorTestSuite-20181121-W3gsdfgs"
]

Test[
	r = RandomInteger[1000, {10, 5, 20}];
	Quiet @ EchoElement[NumericArray[r, "Integer64"], {3, 5, 12}]
	,
	LibraryFunctionError["LIBRARY_USER_ERROR", n_?IntegerQ]
	,
	SameTest -> MatchQ
	,
	TestID->"TensorTestSuite-20181121-asdgsdf"
]

Test[
	Get[FileNameJoin[{currentDirectory, "CreateMatrix", "CreateMatrix.wl"}]];
	CreateMatrix[3,4]
	,
	{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}}
	,
	TestID->"TensorOperations-20150811-Y4J6R0"
]

ExactTest[
	Get[FileNameJoin[{currentDirectory, "MeanValue", "MeanValue.wl"}]];
	MeanValue[{2.2,3.3,4.4}]
	,
	3.3
	,
	TestID->"TensorOperations-20150817-A4F7C6"
]

Test[
	Get[FileNameJoin[{currentDirectory, "Integer","IntegerMatrixTranspose","IntegerMatrixTranspose.wl"}]];
	IntegerMatrixTranspose[{{1,2,3},{4,5,6}}]
	,
	{{1,4},{2,5},{3,6}}
	,
	TestID->"TensorOperations-20150817-L0F1J5"
]

(********************Scalar operations on tensors****************************************)
Test[
	Get[FileNameJoin[{currentDirectory, "ScalarOperations","ScalarOperations.wl"}]];
	getNthRealFromTR1[{1,2,3,4.7},4]
	,
	4.7
	,
	TestID->"TensorOperations-20150817-F9U7F4"
]

ExactTest[
	getNthRealFromTR1[{1,2,3,4.7},100]
	,
	LibraryFunctionError["LIBRARY_USER_ERROR", n_] /; n < 0 (* even though we know what the error is, we cannot predict the error code *)
	,
	LibraryFunction::rterr
	,
	SameTest -> MatchQ
	,
	TestID->"TensorOperations-20150817-Z2M1Q2"
]

ExactTest[
	getNthRealFromTR2[{{1,2,3},{4,5,6}},1,3]
	,
	3.
	,
	TestID->"TensorOperations-20150817-N1I3G8"
]

ExactTest[
	getNthIntegerFromTR2[{{1,2,3},{4,5,6}},1,3]
	,
	3
	,
	TestID->"TensorOperations-20150817-J6E5K2"
]

Test[
	setNthIntegerT[7]
	,
	{2, 4, 6, 8, 10, 12, 14}
	,
	TestID->"TensorOperations-20150818-Y6F8K2"
]

(********************Calling tensor apis with shared data****************************************)
Test[
	Get[FileNameJoin[{currentDirectory, "SharedData","SharedData.wl"}]];
	loadRealArray[Developer`ToPackedArray[{2.3,4.5}]];
	getRealArray[]
	,
	{2.3,4.5}
	,
	TestID->"TensorOperations-20150819-F5H0C3"
]

Test[
	doubleRealArray[]
	,
	{4.6,9.}
	,
	TestID->"TensorOperations-20150819-R4E5S2"
]

Test[
	unloadRealArray[]
	,
	0
	,
	TestID->"TensorOperations-20150819-D8C1Y0"
]


Test[
	copyShared[RandomReal[1., {3, 5}]]
	,
	110
	,
	TestID->"TensorOperations-20150831-L0U3V3"
]

EndRequirement[]