/**
 * @file
 * @brief   GenericTensor definition and implementation
 */

#ifndef LLU_INCLUDE_LLU_CONTAINERS_GENERIC_TENSOR
#define LLU_INCLUDE_LLU_CONTAINERS_GENERIC_TENSOR

#include "LLU/Containers/Generic/Base.hpp"
#include "LLU/Containers/Interfaces.h"

namespace LLU {

	template<class PassingMode>
	class MContainer<MArgumentType::Tensor, PassingMode>;

	/// MContainer specialization for MTensor is called GenericTensor
	template<class PassingMode>
	using GenericTensor = MContainer<MArgumentType::Tensor, PassingMode>;
	
	/**
	 *  @brief  MContainer specialization for MTensor
	 *  @tparam PassingMode - passing policy
	 */
	template<class PassingMode>
	class MContainer<MArgumentType::Tensor, PassingMode> : public TensorInterface, public MContainerBase<MArgumentType::Tensor, PassingMode> {
	public:
		/// Inherit constructors from MContainerBase
		using MContainerBase<MArgumentType::Tensor, PassingMode>::MContainerBase;

		/// Default constructor, the MContainer does not manage any instance of MTensor.
		MContainer() = default;

		/**
		 * @brief   Create GenericTensor of given type and shape
		 * @param   type - new GenericTensor type (MType_Integer, MType_Real or MType_Complex)
		 * @param   rank - new GenericTensor rank
		 * @param   dims - new GenericTensor dimensions
		 * @see     <http://reference.wolfram.com/language/LibraryLink/ref/callback/MTensor_new.html>
		 */
		MContainer(mint type, mint rank, const mint* dims) {
			RawContainer tmp {};
			if (LibraryData::API()->MTensor_new(type, rank, dims, &tmp)) {
				ErrorManager::throwException(ErrorName::TensorNewError);
			}
			this->setContainer(tmp);
		}

		/**
		 * @brief   Create GenericTensor from another GenericTensor with different passing mode.
		 * @tparam  P - some passing mode
		 * @param   mc - different GenericTensor
		 */
		template<class P>
		explicit MContainer(const MContainer<MArgumentType::Tensor, P>& mc) : Base(mc) {}

		MContainer(const MContainer& mc) = default;

		MContainer(MContainer&& mc) noexcept = default;

		MContainer& operator=(const MContainer&) = default;

		MContainer& operator=(MContainer&& mc) noexcept = default;

		/**
		 * @brief   Assign a GenericTensor with different passing mode.
		 * @tparam  P - some passing mode
		 * @param   mc - different GenericTensor
		 * @return  this
		 */
		template<class P>
		MContainer& operator=(const MContainer<MArgumentType::Tensor, P>& mc) {
			Base::operator=(mc);
			return *this;
		}

		/// Destructor which triggers the appropriate cleanup action which depends on the PassingMode
		~MContainer() {
			this->cleanup();
		};

		/// @copydoc TensorInterface::getRank()
		mint getRank() const override {
			return LibraryData::API()->MTensor_getRank(this->getContainer());
		}

		/// @copydoc TensorInterface::getDimensions()
		mint const* getDimensions() const override {
			return LibraryData::API()->MTensor_getDimensions(this->getContainer());
		}

		/// @copydoc TensorInterface::getFlattenedLength()
		mint getFlattenedLength() const override {
			return LibraryData::API()->MTensor_getFlattenedLength(this->getContainer());
		}

		/// @copydoc TensorInterface::type()
		mint type() const override {
			return LibraryData::API()->MTensor_getType(this->getContainer());
		}

		/// @copydoc TensorInterface::rawData()
		void* rawData() const override {
			switch (type()) {
				case MType_Integer: return LibraryData::API()->MTensor_getIntegerData(this->getContainer());
				case MType_Real: return LibraryData::API()->MTensor_getRealData(this->getContainer());
				case MType_Complex: return LibraryData::API()->MTensor_getComplexData(this->getContainer());
				default: ErrorManager::throwException(ErrorName::TensorTypeError);
			}
		}
	private:
		using Base = MContainerBase<MArgumentType::Tensor, PassingMode>;
		using RawContainer = typename Base::Container;

		/**
		 * @copydoc MContainerBase::shareCount()
		 * @see 	<http://reference.wolfram.com/language/LibraryLink/ref/callback/MTensor_shareCount.html>
		 */
		mint shareCountImpl() const noexcept override {
			return LibraryData::API()->MTensor_shareCount(this->getContainer());
		}

		/**
		 *   @copydoc   MContainerBase::disown()
		 *   @see 		<http://reference.wolfram.com/language/LibraryLink/ref/callback/MTensor_disown.html>
		 **/
		void disownImpl() const noexcept override {
			LibraryData::API()->MTensor_disown(this->getContainer());
		}

		/**
		 *   @copydoc   MContainerBase::free()
		 *   @see 		<http://reference.wolfram.com/language/LibraryLink/ref/callback/MTensor_free.html>
		 **/
		void freeImpl() const noexcept override {
			LibraryData::API()->MTensor_free(this->getContainer());
		}

		/**
		 *   @copydoc   MContainerBase::pass
		 **/
		void passImpl(MArgument& res) const noexcept override {
			MArgument_setMTensor(res, this->getContainer());
		}

		/**
		 *   @copydoc   MContainerBase::clone()
		 *   @see 		<http://reference.wolfram.com/language/LibraryLink/ref/callback/MTensor_clone.html>
		 **/
		RawContainer cloneImpl() const override {
			RawContainer tmp {};
			if (LibraryData::API()->MTensor_clone(this->getContainer(), &tmp)) {
				ErrorManager::throwException(ErrorName::TensorCloneError);
			}
			return tmp;
		}
	};

}

#endif	  // LLU_INCLUDE_LLU_CONTAINERS_GENERIC_TENSOR
