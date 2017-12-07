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
 * @date 2017
 * Unit tests for the iulia function inliner.
 */

#include <test/libjulia/Common.h>

#include <libjulia/optimiser/FullInliner.h>
#include <libjulia/optimiser/FunctionHoister.h>

#include <libsolidity/inlineasm/AsmPrinter.h>

#include <boost/test/unit_test.hpp>

#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string/join.hpp>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::julia::test;
using namespace dev::solidity::assembly;

namespace
{
string fullInline(string const& _source, bool _julia = true)
{
	Block ast = disambiguate(_source, _julia);
	(FunctionHoister{})(ast);
	FullInliner(ast).run();
	return AsmPrinter(_julia)(ast);
}
}

BOOST_AUTO_TEST_SUITE(IuliaFullInliner)

BOOST_AUTO_TEST_CASE(simple)
{
	BOOST_CHECK_EQUAL(
		fullInline(R"({
			function f(a) -> x { let r := mul(a, a) x := add(r, r) }
			let y := add(f(sload(mload(2))), mload(7))
		})", false),
		format(R"({
			{
				let _1 := mload(7)
				let f_a := sload(mload(2))
				let f_x
				{
					let f_r := mul(f_a, f_a)
					f_x := add(f_r, f_r)
				}
				let y := add(f_x, _1)
			}
			function f(a) -> x
			{
				let r := mul(a, a)
				x := add(r, r)
			}
		})", false)
	);
}

BOOST_AUTO_TEST_CASE(multi_fun)
{
	BOOST_CHECK_EQUAL(
		fullInline(R"({
			function f(a) -> x { x := add(a, a) }
			function g(b, c) -> y { y := mul(mload(c), f(b)) }
			let y := g(f(3), 7)
		})", false),
		format(R"({
			{
				let g_c := 7
				let f_a_1 := 3
				let f_x_1
				{ f_x_1 := add(f_a_1, f_a_1) }
				let g_y
				{
					let g_f_a := f_x_1
					let g_f_x
					{
						g_f_x := add(g_f_a, g_f_a)
					}
					g_y := mul(mload(g_c), g_f_x)
				}
				let y_1 := g_y
			}
			function f(a) -> x
			{
				x := add(a, a)
			}
			function g(b, c) -> y
			{
				let f_a := b
				let f_x
				{
					f_x := add(f_a, f_a)
				}
				y := mul(mload(c), f_x)
			}
		})", false)
	);
}

BOOST_AUTO_TEST_CASE(move_up_rightwards_arguments)
{
	BOOST_CHECK_EQUAL(
		fullInline(R"({
			function f(a, b, c) -> x { x := add(a, b) x := mul(x, c) }
			let y := add(mload(1), add(f(mload(2), mload(3), mload(4)), mload(5)))
		})", false),
		format(R"({
			{
				let _1 := mload(5)
				let f_c := mload(4)
				let f_b := mload(3)
				let f_a := mload(2)
				let f_x
				{
					f_x := add(f_a, f_b)
					f_x := mul(f_x, f_c)
				}
				let y := add(mload(1), add(f_x, _1))
			}
			function f(a, b, c) -> x
			{
				x := add(a, b)
				x := mul(x, c)
			}
		})", false)
	);
}

BOOST_AUTO_TEST_SUITE_END()
