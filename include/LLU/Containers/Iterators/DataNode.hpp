/**
 * @file	DataNode.hpp
 * @author	Rafal Chojna <rafalc@wolfram.com>
 * @date	May 06, 2020
 * @brief
 */
#ifndef LIBRARYLINKUTILITIES_CONTAINERS_ITERATORS_DATANODE_HPP
#define LIBRARYLINKUTILITIES_CONTAINERS_ITERATORS_DATANODE_HPP

#include <type_traits>

#include <LLU/Containers/Generic/DataStore.hpp>
#include <LLU/TypedMArgument.h>

namespace LLU {

	/**
	 * @class	DataNode
	 * @brief	Wrapper over DataStoreNode structure from LibraryLink.
	 * 			It stores node name in std::string and node value as MArgument, getters for both are provided.
	 */
	template<typename T>
	class DataNode {
		static constexpr bool isGeneric = std::is_same_v<T, Argument::TypedArgument>;
		static_assert(Argument::WrapperQ<T>, "DataNode type is not a valid MArgument wrapper type.");

	public:
		/**
		 * @brief 	Create DataNode from raw DataStoreNode structure
		 * @param 	dsn - raw node
		 */
		explicit DataNode(DataStoreNode dsn);

		explicit DataNode(GenericDataNode gn);

		/**
		 * @brief 	Get node value
		 * @return 	Returns a reference to node value
		 */
		T& value() {
			return nodeArg;
		}

		/**
		 * @brief 	Get node value
		 * @return 	Returns a reference to node value
		 */
		const T& value() const {
			return nodeArg;
		}

		[[nodiscard]] std::string_view name() const {
			return node.name();
		}

		[[nodiscard]] bool hasNext() const {
			return static_cast<bool>(node.next());
		}

		template<typename U = T>
		DataNode<U> next() const {
			return {node.next()};
		}

		/**
		 * @brief 	Get the actual type of node value stored in MArgument.
		 * 			This is useful when working on a "generic" DataList of type MArgumentType::MArgument, otherwise it should always return MArgType
		 * @return	Actual type of node value
		 */
		MArgumentType valueType() noexcept {
			return node.type();
		}

		template <std::size_t N>
		decltype(auto) get() {
			static_assert(N < 2, "Bad structure binding attempt to a DataNode.");
			if constexpr (N == 0) {
				return name();
			} else {
				return (nodeArg);
			}
		}

	private:
		GenericDataNode node;
		T nodeArg;
	};

	/* Definitions od DataNode methods */
	template<typename T>
	DataNode<T>::DataNode(DataStoreNode dsn) : DataNode(GenericDataNode {dsn}) {}

	template<typename T>
	DataNode<T>::DataNode(GenericDataNode gn) : node {gn} {
		if (!node) {
			ErrorManager::throwException(ErrorName::DLNullRawNode);
		}
		if constexpr (!isGeneric) {
			nodeArg = std::move(node.as<T>());
		} else{
			nodeArg = std::move(node.value());
		}
	}


}/* namespace LLU */

namespace std {
	template<typename T>
	struct tuple_size<LLU::DataNode<T>> : std::integral_constant<std::size_t, 2> {};

	template<std::size_t N, typename T>
	struct tuple_element<N, LLU::DataNode<T>> {
		using type = decltype(std::declval<LLU::DataNode<T>>().template get<N>());
	};
}

#endif	  // LIBRARYLINKUTILITIES_CONTAINERS_ITERATORS_DATANODE_HPP