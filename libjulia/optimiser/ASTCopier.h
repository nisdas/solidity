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
 * Creates an independent copy of an AST, renaming identifiers to be unique.
 */

#pragma once

#include <libjulia/ASTDataForward.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <vector>
#include <set>
#include <memory>

namespace dev
{
namespace julia
{

/**
 * Creates a copy of a iulia AST potentially replacing identifier names.
 * Base class to be extended.
 */
class ASTCopier: public boost::static_visitor<Statement>
{
public:
	virtual Statement operator()(Literal const& _literal);
	virtual Statement operator()(Instruction const& _instruction);
	virtual Statement operator()(Identifier const& _identifier);
	virtual Statement operator()(FunctionalInstruction const& _instr);
	virtual Statement operator()(FunctionCall const&);
	virtual Statement operator()(Label const& _label);
	virtual Statement operator()(StackAssignment const& _assignment);
	virtual Statement operator()(Assignment const& _assignment);
	virtual Statement operator()(VariableDeclaration const& _varDecl);
	virtual Statement operator()(If const& _if);
	virtual Statement operator()(Switch const& _switch);
	virtual Statement operator()(FunctionDefinition const&);
	virtual Statement operator()(ForLoop const&);
	virtual Statement operator()(Block const& _block);

	virtual Statement translate(Statement const& _statement);

protected:
	template <typename T>
	std::vector<T> translateVector(std::vector<T> const& _values);

	template <typename T>
	std::shared_ptr<T> translate(std::shared_ptr<T> const& _v)
	{
		return _v ? std::make_shared<T>(translate(*_v)) : nullptr;
	}
	Block translate(Block const& _block);
	Case translate(Case const& _case);
	Identifier translate(Identifier const& _identifier);
	Literal translate(Literal const& _literal);
	TypedName translate(TypedName const& _typedName);

	virtual void enterScope(Block const&) { }
	virtual void leaveScope(Block const&) { }
	virtual void enterFunction(FunctionDefinition const&) { }
	virtual void leaveFunction(FunctionDefinition const&) { }
	virtual std::string translateIdentifier(std::string const& _name) { return _name; }
};

template <typename T>
std::vector<T> ASTCopier::translateVector(std::vector<T> const& _values)
{
	std::vector<T> translated;
	for (auto const& v: _values)
		translated.emplace_back(translate(v));
	return translated;
}

}
}
