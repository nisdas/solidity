/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Optimiser component that performs function inlining.
 */

#include <libjulia/optimiser/FunctionalInliner.h>

#include <libjulia/optimiser/InlinableFunctionFilter.h>
#include <libjulia/optimiser/Substitution.h>

#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/inlineasm/AsmScope.h>

#include <libevmasm/SemanticInformation.h>

#include <libsolidity/interface/Exceptions.h>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::solidity;

void FunctionalInliner::run()
{
	InlinableFunctionFilter filter;
	filter(m_block);
	m_inlinableFunctions = filter.inlinableFunctions();

	(*this)(m_block);
}

void FunctionalInliner::operator()(FunctionalInstruction& _instr)
{
	bool movable = eth::SemanticInformation::movable(_instr.instruction);

	ASTModifier::operator()(_instr);

	if (!movable)
		m_isMovable = false;
}

void FunctionalInliner::operator()(FunctionCall& _call)
{
	ASTModifier::operator()(_call);
	// We do not set movable to false, it will be done in ::visit later.
	// If we set it to false here, we cannot detect the internal movability.
}

void FunctionalInliner::visit(Statement& _statement)
{
	if (_statement.type() == typeid(FunctionCall))
	{
		FunctionCall& funCall = boost::get<FunctionCall>(_statement);
		m_isMovable = true;
		ASTModifier::visit(_statement);

		if (m_isMovable && m_inlinableFunctions.count(funCall.functionName.name))
		{
			FunctionDefinition const& fun = *m_inlinableFunctions.at(funCall.functionName.name);
			map<string, Statement const*> substitutions;
			for (size_t i = 0; i < fun.parameters.size(); ++i)
				substitutions[fun.parameters[i].name] = &funCall.arguments[i];
			_statement = Substitution(substitutions).translate(*boost::get<Assignment>(fun.body.statements.front()).value);

			// TODO actually in the process of inlining, we could also make a function non-inlinable
			// because it could now call itself

			// Movability of this depends on the movability of the replacement,
			// i.e. the movability of the function itself.
			// Perhaps we can just re-run?
			// Let us set it to non-movable for now.

			// If two functions call each other, we have to exit after some iterations.

		}
		m_isMovable = false;
	}
	else
		ASTModifier::visit(_statement);
}
