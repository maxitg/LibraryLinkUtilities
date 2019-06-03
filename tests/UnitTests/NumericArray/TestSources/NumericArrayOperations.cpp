#include <iostream>
#include <numeric>
#include <type_traits>

#include "LLU/LLU.h"
#include "LLU/LibraryLinkFunctionMacro.h"

namespace NA = LibraryLinkUtils::NA;

static MNumericArray shared_numeric = 0;

EXTERN_C DLLEXPORT mint WolframLibrary_getVersion() {
	return WolframLibraryVersion;
}

EXTERN_C DLLEXPORT int WolframLibrary_initialize(WolframLibraryData libData) {
	MArgumentManager::setLibraryData(libData);
	ErrorManager::registerPacletErrors({
		{"InvalidConversionMethod", "NumericArray conversion method `method` is invalid."}
	});
	return 0;
}

LIBRARY_LINK_FUNCTION(echoNumericArray) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnNumericArray(0, [&mngr](auto&& rarray1) {
			auto rarray2(std::move(rarray1));
			mngr.setNumericArray(rarray2);
		});
	}
	catch (const LibraryLinkError& e) {
		err = e.which();
		std::cout << e.what() << std::endl;
	}
	catch (...) {
		err = LLErrorCode::FunctionError;
	}
	return err;
}

/*
 * Numeric array library functions
 */
LIBRARY_LINK_FUNCTION(getNumericArrayLength) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnNumericArray(0, [&mngr](auto&& rarray) {
			mngr.setInteger(rarray.size());
		});
	}
	catch (const LibraryLinkError& e) {
		err = e.which();
	}
	catch (...) {
		err = LLErrorCode::FunctionError;
	}
	return err;
}

LIBRARY_LINK_FUNCTION(getNumericArrayRank) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnNumericArray(0, [&mngr](auto&& rarray) {
			mngr.setInteger(rarray.rank());
		});
	}
	catch (const LibraryLinkError& e) {
		err = e.which();
	}
	catch (...) {
		err = LLErrorCode::FunctionError;
	}
	return err;
}

//create new numeric array
LIBRARY_LINK_FUNCTION(newNumericArray) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		NumericArray<float> ra(0., { 3, 3 });
		mngr.setNumericArray(ra);
	}
	catch (const LibraryLinkError& e) {
		err = e.which();
	}
	catch (...) {
		err = LLErrorCode::FunctionError;
	}
	return err;
}

//clone NumericArray
LIBRARY_LINK_FUNCTION(cloneNumericArray) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnNumericArray(0, [&mngr](auto&& rarray1) {
			using T = typename std::decay_t<decltype(rarray1)>::value_type;
			NumericArray<T, Passing::Manual> rarray2 = rarray1;
			mngr.setNumericArray(rarray2);
		});
	}
	catch (const LibraryLinkError& e) {
		err = e.which();
	}
	catch (...) {
		err = LLErrorCode::FunctionError;
	}
	return err;
}

LIBRARY_LINK_FUNCTION(changeSharedNumericArray) {
	WolframNumericArrayLibrary_Functions numericArrayFunctions = libData->numericarrayLibraryFunctions;
	int err = LIBRARY_NO_ERROR;

	if (shared_numeric) {
		numericArrayFunctions->MNumericArray_disown(shared_numeric);
		shared_numeric = 0;
	}

	shared_numeric = MArgument_getMNumericArray(Args[0]);

	return err;
}

EXTERN_C DLLEXPORT int getSharedNumericArray(WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res) {
	int err = LIBRARY_NO_ERROR;

	if (shared_numeric) {
		MArgument_setMNumericArray(Res, shared_numeric);
	}
	else
		err = LIBRARY_FUNCTION_ERROR;

	return err;
}

struct ZeroReal64 {
	template<typename T, class P>
	void operator()(NumericArray<T, P>, MArgumentManager&) {
		ErrorManager::throwException(LLErrorName::FunctionError);
	}

	template<class P>
	void operator()(NumericArray<double, P>& ra, MArgumentManager& mngr) {
		std::fill(ra.begin(), ra.end(), 0.0);
		mngr.setNumericArray(ra);
	}
};

//reset NumericArray
EXTERN_C DLLEXPORT int numericZeroData(WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnNumericArray<ZeroReal64>(0, mngr);
	}
	catch (const LibraryLinkError& e) {
		err = e.which();
	}
	catch (...) {
		err = LLErrorCode::FunctionError;
	}
	return err;
}

struct AccumulateIntegers {
	template<typename T, class P>
	std::enable_if_t<!std::is_integral<T>::value> operator()(NumericArray<T, P>, MArgumentManager&) {
		ErrorManager::throwException(LLErrorName::FunctionError);
	}

	template<typename T, class P>
	std::enable_if_t<std::is_integral<T>::value> operator()(NumericArray<T, P> ra, MArgumentManager& mngr) {
		auto result = std::accumulate(ra.begin(), ra.end(), static_cast<T>(0));
		mngr.setInteger(result);
	}
};

//sum elements of a NumericArray but only if it is of integer type
EXTERN_C DLLEXPORT int accumulateIntegers(WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnNumericArray<AccumulateIntegers>(0, mngr);
	}
	catch (const LibraryLinkError& e) {
		err = e.which();
	}
	catch (...) {
		err = LLErrorCode::FunctionError;
	}
	return err;
}

// check if conversion methods are mapped correctly
LIBRARY_LINK_FUNCTION(convertMethodName) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		auto method = mngr.getInteger<NA::ConversionMethod>(0);
		std::string methodStr;
		switch (method) {
			case NA::ConversionMethod::Check: methodStr = "Check";
				break;
			case NA::ConversionMethod::ClipCheck: methodStr = "ClipCheck";
				break;
			case NA::ConversionMethod::Coerce: methodStr = "Coerce";
				break;
			case NA::ConversionMethod::ClipCoerce: methodStr = "ClipCoerce";
				break;
			case NA::ConversionMethod::Round: methodStr = "Round";
				break;
			case NA::ConversionMethod::ClipRound: methodStr = "ClipRound";
				break;
			case NA::ConversionMethod::Scale: methodStr = "Scale";
				break;
			case NA::ConversionMethod::ClipScale: methodStr = "ClipScale";
				break;
			default:
				ErrorManager::throwException("InvalidConversionMethod", static_cast<int>(method));
		}
		mngr.setString(std::move(methodStr));
	}
	catch (const LibraryLinkError& e) {
		err = e.which();
	}
	catch (...) {
		err = LLErrorCode::FunctionError;
	}
	return err;
}

// convert NumericArray
LIBRARY_LINK_FUNCTION(convert) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnNumericArray(0, [&mngr](auto&& numArr) {
			NumericArray<std::uint16_t> converted { numArr, mngr.getInteger<NA::ConversionMethod>(1), mngr.getReal(2) };
			mngr.setNumericArray(converted);
		});
	}
	catch (const LibraryLinkError& e) {
		err = e.which();
	}
	catch (...) {
		err = LLErrorCode::FunctionError;
	}
	return err;
}
