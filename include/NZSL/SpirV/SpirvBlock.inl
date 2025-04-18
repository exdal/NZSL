// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl
{
	inline SpirvBlock::SpirvBlock(SpirvWriter& writer) :
	m_isTerminated(false)
	{
		m_labelId = writer.AllocateResultId();
		Append(SpirvOp::OpLabel, m_labelId);
	}

	inline std::size_t SpirvBlock::Append(SpirvOp opcode, const OpSize& wordCount)
	{
		HandleSpirvOp(opcode);

		return SpirvSectionBase::Append(opcode, wordCount);
	}

	template<typename... Args>
	std::size_t SpirvBlock::Append(SpirvOp opcode, Args&&... args)
	{
		HandleSpirvOp(opcode);

		return SpirvSectionBase::Append(opcode, std::forward<Args>(args)...);
	}

	template<typename F>
	std::size_t SpirvBlock::AppendVariadic(SpirvOp opcode, F&& callback)
	{
		HandleSpirvOp(opcode);

		return SpirvSectionBase::AppendVariadic(opcode, std::forward<F>(callback));
	}

	inline std::uint32_t SpirvBlock::GetLabelId() const
	{
		return m_labelId;
	}

	inline bool SpirvBlock::IsTerminated() const
	{
		return m_isTerminated;
	}

	inline bool SpirvBlock::IsTerminationInstruction(SpirvOp op)
	{
		switch (op)
		{
			case SpirvOp::OpBranch:
			case SpirvOp::OpBranchConditional:
			case SpirvOp::OpKill:
			case SpirvOp::OpReturn:
			case SpirvOp::OpReturnValue:
			case SpirvOp::OpSwitch:
			case SpirvOp::OpUnreachable:
				return true;

			default:
				return false;
		}
	}

	inline void SpirvBlock::HandleSpirvOp(SpirvOp op)
	{
		assert(!m_isTerminated);
		if (IsTerminationInstruction(op))
			m_isTerminated = true;
	}
}
