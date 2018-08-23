/**
 * @file	LibDataHolder.h
 * @author	Rafal Chojna <rafalc@wolfram.com>
 * @date	14/06/2018
 *
 * @brief	Definition of the LibDataHolder class.
 *
 */
#ifndef LLUTILS_LIBDATAHOLDER_H
#define LLUTILS_LIBDATAHOLDER_H

#include <memory>

#include "WolframLibrary.h"
#include "WolframImageLibrary.h"
#include "WolframRawArrayLibrary.h"

namespace LibraryLinkUtils {

	/**
	 * @struct 	LibDataHolder
	 * @brief	This structure offers a static copy of WolframLibData accessible throughout the whole life of the DLL.
	 */
	struct LibDataHolder {
		/**
		 *   @brief         Set WolframLibraryData structure as static member of LibDataHolder. Call this function in WolframLibrary_initialize.
		 *   @param[in]     ld - WolframLibraryData passed to every library function via LibraryLink
		 *   @warning		This function must be called before constructing the first MArgumentManager unless you use a constructor that takes WolframLibraryData as argument
		 **/
		static void setLibraryData(WolframLibraryData ld);

		/**
		 *   @brief         Get currently owned WolframLibraryData.
		 *   @return     	a non-owning pointer to current instance of st_WolframLibraryData stored in LibDataHolder or nullptr in case there is none
		 **/
		static WolframLibraryData getLibraryData() noexcept;

	protected:
		static std::unique_ptr<st_WolframLibraryData> libData;
		static WolframRawArrayLibrary_Functions raFuns;
		static WolframImageLibrary_Functions imgFuns;
	};

}

#endif //LLUTILS_LIBDATAHOLDER_H