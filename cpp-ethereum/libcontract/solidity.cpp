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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity commandline compiler.
 */

#include <clocale>
#include <iostream>
#include <boost/exception/all.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/JSON.h>

#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/Version.h>


using namespace std;
using namespace dev;
using namespace solidity;

//extern string compileSingle(string const& _input, bool _optimize);

namespace dev
{
namespace eth
{

extern "C" {
/// Callback used to retrieve additional source files. "Returns" two pointers that should be
/// heap-allocated and are free'd by the caller.
typedef void (*CStyleReadFileCallback)(char const* _path, char** o_contents, char** o_error);
}

ReadCallback::Callback wrapReadCallback(CStyleReadFileCallback _readCallback = nullptr)
{
	ReadCallback::Callback readCallback;
	if (_readCallback)
	{
		readCallback = [=](string const& _path)
		{
			char* contents_c = nullptr;
			char* error_c = nullptr;
			_readCallback(_path.c_str(), &contents_c, &error_c);
			ReadCallback::Result result;
			result.success = true;
			if (!contents_c && !error_c)
			{
				result.success = false;
				result.responseOrErrorMessage = "File not found.";
			}
			if (contents_c)
			{
				result.success = true;
				result.responseOrErrorMessage = string(contents_c);
				free(contents_c);
			}
			if (error_c)
			{
				result.success = false;
				result.responseOrErrorMessage = string(error_c);
				free(error_c);
			}
			return result;
		};
	}
	return readCallback;
}

string compile(StringMap const& _sources, bool _optimize, CStyleReadFileCallback _readCallback)
{
	/// create new JSON input format
	Json::Value input = Json::objectValue;
	input["language"] = "Solidity";
	input["sources"] = Json::objectValue;
	for (auto const& source: _sources)
	{
		input["sources"][source.first] = Json::objectValue;
		input["sources"][source.first]["content"] = source.second;
	}
	input["settings"] = Json::objectValue;
	input["settings"]["optimizer"] = Json::objectValue;
	input["settings"]["optimizer"]["enabled"] = _optimize;
	input["settings"]["optimizer"]["runs"] = 200;

	StandardCompiler compiler(wrapReadCallback(_readCallback));
	Json::Value ret = compiler.compile(input);

	if (ret.isMember("contracts")){
		for (auto const& sourceName: ret["contracts"].getMemberNames()){
			for (auto const& contractName: ret["contracts"][sourceName].getMemberNames()){
				Json::Value contractInput = ret["contracts"][sourceName][contractName];
				return contractInput["evm"]["deployedBytecode"]["object"].asString();
			}
			
		}
	}

	return "";
}


string compileSingle(string const& _input, bool _optimize)
{
	StringMap sources;
	sources[""] = _input;
	return compile(sources, _optimize, nullptr);
}

std::string compileCode(std::string code)
{
	string json = compileSingle(code, false);
	cout << "json=" << json << endl;
	return json;	
}

std::string compileFile(std::string name)
{
	string code = contentsString(name);
	cout << "name=" << name << ",code:"  << endl << code << endl;
	if(code.empty())
		return std::string();
	
	return compileCode(code);
}



}
}
