#ifndef CODEGEN_H_
#define CODEGEN_H_

#include "scope.h"
#include <vector>

class CodeGen {
public:
	CodeGen() {}

	~CodeGen() {
		delete _globalScope;
	}

	void GenCode(TreeNode* node) {
		rootNodeAST = node;

		_globalScope = new Scope();
		_currentScope = _globalScope;
		compileNode(rootNodeAST);

		_currentEmitIndex = 0;
		_endEmitIndex = 0;
	}

	void Emit(const std::string& code) {
		_emittedCode.push_back(code);
	}

	void BeginEmit() {
		_currentEmitIndex = _emittedCode.size();
	}

	void EndEmit() {
		_endEmitIndex = _emittedCode.size() - 1;
	}

	void pasteEmittedCodeHere() {
		std::vector<std::string> tmpEmit;
		std::vector<std::string> new_emittedCodeList;

		for (unsigned int i = 0; i < _emittedCode.size(); ++i) {
			if (i >= _currentEmitIndex && i <= _endEmitIndex)
				continue;

			new_emittedCodeList.push_back(_emittedCode[i]);
		}

		for (int i = _currentEmitIndex; i < _endEmitIndex + 1; ++i)
			new_emittedCodeList.push_back(_emittedCode.at(i));

		_emittedCode = new_emittedCodeList;
	}

	void printGeneratedCode() {
		for (std::string s : _emittedCode)
			std::cout << s << std::endl;
	}

private:
	void compileNode(TreeNode* node) {
		switch (node->m_nodeType) {
		case NodeType::NODE_COMPILATION_UNIT:
			for (TreeNode* child : node->m_children)
				compileNode(child);
			break;

		case NodeType::NODE_FUNCTIONDECL:

			//Извлечь имя и тип функции
			_currentScope->addSymbol(new Symbol(node->getChild(0)->getChildIdentifier(1), SymbolType::FUNCTION, node->getChild(0)->getChild(0)->getChild(0)->m_nodeType));
			Emit("_" + _currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChildIdentifier(1))->_symbolName + ":");

			//Комфиляция блока кода
			BeginEmit();
			_currentScope = _currentScope->createScope();
			compileNode(node->getChild(1));
			EndEmit();

			Emit("push ebp");																	//Сохранить указатель стека
			Emit("mov ebp, esp");																//ebp теперь указывает на вершину стека
			Emit("sub esp, " + std::to_string(_currentScope->symbolTable->getSize() * 4));    //Пространство выделенное в стеке для локальных переменных

			pasteEmittedCodeHere();

			Emit("mov esp, ebp");
			Emit("pop ebp");
			Emit("ret");

			_currentScope = _currentScope->leaveScope();

			break;

		case NodeType::NODE_BLOCK:
			for (TreeNode* child : node->m_children)
				compileNode(child);
			break;

		case NodeType::NODE_STATEMENT:
			for (TreeNode* child : node->m_children)
				compileNode(child);
			break;

		case NodeType::NODE_DECLARATION:
			_currentScope->addSymbol(new Symbol(node->getChild(1)->m_token._identifier));

			for (TreeNode* child : node->m_children)
				compileNode(child);

			Emit("mov [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(1)->m_token._identifier)->_address) + "], eax");

			break;

		case NodeType::NODE_OP_ASSIGN:
			if (node->getChild(0)->m_nodeType == NodeType::NODE_FACTOR) {
				if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
					Emit("mov eax, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->m_token._identifier)->_address) + "]");
				}
			}
			else
				compileNode(node->getChild(0));

			break;

		case NodeType::NODE_INITIALIZER:
			compileNode(node->getChild(0));
			break;

		case NODE_ASSEMBLY_LINE:
			Emit(node->m_token._identifier);
			break;


		case NodeType::NODE_BINARY_ADD:

			if (node->getChild(0)->m_nodeType == NodeType::NODE_FACTOR) {
				if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
					Emit("mov eax, " + node->getChild(0)->getChild(0)->m_token.getStringValue());
				}
				else if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
					Emit("mov eax, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->m_token._identifier)->_address) + "]");
				}
			}
			else {
				compileNode(node->getChild(0));
			}

			if (node->getChild(1)->m_nodeType == NodeType::NODE_FACTOR) {
				if (node->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
					Emit("mov ebx, " + node->getChild(1)->getChild(0)->m_token.getStringValue());
				}
				else if (node->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
					Emit("mov ebx, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(1)->getChild(0)->m_token._identifier)->_address) + "]");
				}
			}
			else {
				compileNode(node->getChild(1));
			}

			Emit("add eax, ebx");

			break;

		case NodeType::NODE_PRINT:
			if (node->getChild(0)->m_nodeType == NodeType::NODE_STRING) {
				Emit("mov ecx, " + node->getChild(0)->m_token.getStringValue());
				Emit("mov ebx, 1");
				Emit("mov eax, 4");
				Emit("int 0x80");
			}
			else if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
				Emit("mov ecx, " + node->getChild(0)->getChild(0)->m_token.getStringValue());
				Emit("mov ebx, 1");
				Emit("mov eax, 4");
				Emit("int 0x80");
			}
			else if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
				Emit("mov ecx, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->m_token._identifier)->_address) + "]");
				Emit("mov ebx, 1");
				Emit("mov eax, 4");
				Emit("int 0x80");
			}
			break;

		case NodeType::NODE_MULTIPLICATION:
			if (node->getChild(0)->m_nodeType == NodeType::NODE_FACTOR) {
				if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
					Emit("mov eax, " + node->getChild(0)->getChild(0)->m_token.getStringValue());
				}
				else if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
					Emit("mov eax, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->m_token._identifier)->_address) + "]");
				}
			}
			else {
				compileNode(node->getChild(0));
			}

			if (node->getChild(1)->m_nodeType == NodeType::NODE_FACTOR) {
				if (node->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
					Emit("mov ebx, " + node->getChild(1)->getChild(0)->m_token.getStringValue());
				}
				else if (node->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
					Emit("mov ebx, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(1)->getChild(0)->m_token._identifier)->_address) + "]");
				}
			}
			else {
				compileNode(node->getChild(1));
			}

			Emit("mul ebx");
			break;

		case NodeType::NODE_FUNCARGS:
			for (TreeNode* child : node->m_children) {
				if (child->m_nodeType == NodeType::NODE_IDENTIFIER) {
					Symbol* s = _currentScope->symbolTable->getSymbolbyName(child->m_token._identifier);
					Emit("mov eax, [ebp-" + std::to_string(s->_address) + "]");
					Emit("push eax");
				}
				else if (child->m_nodeType == NodeType::NODE_DIGIT) {
					Emit("push " + std::to_string(child->m_token._value.uintvalue));
				}
			}
			break;

		case NodeType::NODE_FUNCCALL: {
			Symbol* s = _currentScope->parentScope->symbolTable->getSymbolbyName(node->getChildIdentifier(0));

			//Рекурсивный вызов, если s сейчас не null
			if (s == nullptr)
				s = _currentScope->symbolTable->getSymbolbyName(node->getChildIdentifier(0));

			if (node->getChild(1)->m_nodeType == NodeType::NODE_FUNCARGS) {
				compileNode(node->getChild(1));
			}

			//Функция не найдена
			if (s == nullptr) {
				std::cout << "Function '" + node->getChildIdentifier(0) + "' not declared!" << std::endl;
				return;
			}

			Emit("call _" + s->_symbolName);
		}
									break;

		default:
			for (TreeNode* child : node->m_children)
				compileNode(child);
			break;
		}
	}

	TreeNode* rootNodeAST;
	Scope* _globalScope;
	Scope* _currentScope;

	std::vector<std::string> _emittedCode;
	unsigned int _currentEmitIndex, _endEmitIndex;
};

#endif