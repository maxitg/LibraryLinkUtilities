/**
 * @file	MArgument.h
 * @author	Rafal Chojna <rafalc@wolfram.com>
 * @date	September 01, 2018
 * @brief	Template class and utilities to work with MArgument in type-safe manner.
 */

#ifndef LLUTILS_MARGUMENT_H
#define LLUTILS_MARGUMENT_H

#include <string>
#include <variant>

#include "LLU/LibraryData.h"
#include "LLU/Utilities.hpp"

namespace LLU {

	/**
	 * @brief Strongly type enum with possible types of data stored in MArgument.
	 */
	enum class MArgumentType {
		MArgument = MType_Undef,
		Boolean = MType_Boolean,
		Integer = MType_Integer,
		Real = MType_Real,
		Complex = MType_Complex,
		Tensor = MType_Tensor,
		SparseArray = MType_SparseArray,
		NumericArray = MType_NumericArray,
		Image = MType_Image,
		UTF8String = MType_UTF8String,
		DataStore = MType_DataStore
	};

	namespace Argument {
		using PrimitiveAny = std::variant<std::monostate, mbool, mint, mreal, mcomplex, MTensor, MSparseArray, MNumericArray, MImage, char*, DataStore>;

		template<typename T>
		constexpr MArgumentType PrimitiveIndex = static_cast<MArgumentType>(variant_index<PrimitiveAny, T>());

		template<typename T>
		constexpr bool PrimitiveQ = (variant_index<PrimitiveAny, T>() < std::variant_size_v<PrimitiveAny>);

		/**
		 * @brief 	Type alias that binds given MArgumentType (enumerated value) to the corresponding type of MArgument.
		 * @tparam 	T - any value of type MArgumentType
		 */
		template<MArgumentType T>
		using CType = std::conditional_t<T == MArgumentType::MArgument, MArgument, std::variant_alternative_t<static_cast<size_t>(T), PrimitiveAny>>;

		/**
		 * @brief Helper template variable that says if an MArgumentType is a LibraryLink container type
		 */
		template<MArgumentType T>
		constexpr bool ContainerTypeQ = (T == MArgumentType::Tensor || T == MArgumentType::Image || T == MArgumentType::NumericArray ||
										 T == MArgumentType::DataStore || T == MArgumentType::SparseArray);
	}

	/**
	 * @brief Helper template variable that is always false. Useful in metaprogramming.
	 */
	template<MArgumentType T>
	constexpr bool alwaysFalse = false;


	/**
	 * @class	PrimitiveWrapper
	 * @brief	Small class that wraps a reference to MArgument and provides proper API to work with this MArgument.
	 * @tparam 	T - any value of type MArgumentType
	 */
	template<MArgumentType T>
	class PrimitiveWrapper {
	public:
		/// This is the actual type of data stored in \c arg
		using value_type = Argument::CType<T>;

	public:
		/**
		 * @brief 	Construct PrimitiveWrapper from a reference to MArgument
		 * @param 	a - reference to MArgument
		 */
		explicit PrimitiveWrapper(MArgument& a) : arg(a) {}

		/**
		 * @brief 	Get the value stored in MArgument
		 * @return	Reference to the value stored in MArgument
		 */
		value_type& get();

		/**
		 * @brief 	Get the read-only value stored in MArgument
		 * @return 	Const reference to the value stored in MArgument
		 */
		const value_type& get() const;

		/**
		 * @brief 	Get address of the value stored in MArgument. Every MArgument actually stores a pointer.
		 * @return	Pointer to the value stored in MArgument
		 */
		value_type* getAddress() const;

		/**
		 * @brief 	Set new value of type T in MArgument. Memory management is entirely user's responsibility.
		 * @param 	newValue - new value to be written to MArgument \c arg
		 */
		void set(value_type newValue);

		/**
		 * @brief 	Add \c arg to the DataStore ds inside a node named \c name
		 * The optional parameter should only be used by explicit specialization of this function for T equal to MArgumentType::MArgument
		 * @param 	ds - DataStore with values of type T
		 * @param 	name - name for the new node in the DataStore
		 */
		void addToDataStore(DataStore ds, const std::string& name, MArgumentType = T) const;

		/**
		 * @brief 	Add \c val to the DataStore \c ds inside a node named \c name
		 * This is a static method because there is no MArgument involved.
		 * @param 	ds - DataStore with values of type T
		 * @param 	name - name for the new node in the DataStore
		 * @param 	val - value of the new node in the DataStore
		 */
		static void addDataStoreNode(DataStore ds, std::string_view name, value_type val);

		static void addDataStoreNode(DataStore ds, value_type val);

	private:
		MArgument& arg;
	};

	template<MArgumentType T>
	void PrimitiveWrapper<T>::addToDataStore(DataStore ds, const std::string& name, MArgumentType) const {
		addDataStoreNode(ds, name, get());
	}

	/* Explicit specialization for member functions of PrimitiveWrapper class */

#define ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS(ArgType)                                              \
	template<>                                                                                                    \
	auto PrimitiveWrapper<MArgumentType::ArgType>::get()->typename PrimitiveWrapper::value_type&;                                 \
	template<>                                                                                                    \
	auto PrimitiveWrapper<MArgumentType::ArgType>::get() const->const typename PrimitiveWrapper::value_type&;                     \
	template<>                                                                                                    \
	void PrimitiveWrapper<MArgumentType::ArgType>::addDataStoreNode(DataStore ds, std::string_view name, value_type val); \
	template<>                                                                                                    \
	void PrimitiveWrapper<MArgumentType::ArgType>::addDataStoreNode(DataStore ds, value_type val);                        \
	template<>                                                                                                    \
	auto PrimitiveWrapper<MArgumentType::ArgType>::getAddress() const->typename PrimitiveWrapper::value_type*;                    \
	template<>                                                                                                    \
	void PrimitiveWrapper<MArgumentType::ArgType>::set(typename PrimitiveWrapper::value_type newValue);

	ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS(Boolean)
	ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS(Integer)
	ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS(Real)
	ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS(Complex)
	ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS(Tensor)
	ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS(DataStore)
	ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS(SparseArray)
	ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS(NumericArray)
	ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS(Image)
	ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS(UTF8String)

	template<>
	auto PrimitiveWrapper<MArgumentType::MArgument>::get() -> typename PrimitiveWrapper::value_type&;
	template<>
	auto PrimitiveWrapper<MArgumentType::MArgument>::get() const -> const typename PrimitiveWrapper::value_type&;
	template<>
	void PrimitiveWrapper<MArgumentType::MArgument>::addDataStoreNode(DataStore ds, std::string_view name, value_type val) = delete;
	template<>
	void PrimitiveWrapper<MArgumentType::MArgument>::addDataStoreNode(DataStore ds, value_type val) = delete;
	template<>
	auto PrimitiveWrapper<MArgumentType::MArgument>::getAddress() const -> typename PrimitiveWrapper::value_type*;
	template<>
	void PrimitiveWrapper<MArgumentType::MArgument>::set(typename PrimitiveWrapper::value_type newValue);
	template<>
	void PrimitiveWrapper<MArgumentType::MArgument>::addToDataStore(DataStore ds, const std::string& name, MArgumentType) const;

#undef ARGUMENT_DEFINE_SPECIALIZATIONS_OF_MEMBER_FUNCTIONS

}	 // namespace LLU

#endif	  // LLUTILS_MARGUMENT_H
