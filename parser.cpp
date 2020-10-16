#include "parser.h"

Parser::Parser(std::vector<Token> tokens) {
	m_tokenList = tokens;
	m_currentTokenIndex = 0;
}

Parser::~Parser() {
	delete rootNode;
}

TreeNode* Parser::getAST() {
	return rootNode;
}

Token Parser::getCurrent() { //������� ������� �����
	if (m_currentTokenIndex >= m_tokenList.size()) { //���� ������ ���������
		Token t;
		t._type = TokenType::T_EOF; //�������� EOF-�����
		return t;
	}

	return m_tokenList[m_currentTokenIndex];
}

Token Parser::getNext() { //������� ��������� �����
	if (m_currentTokenIndex > m_tokenList.size() - 1) { //���� ������ ���������
		Token t;
		t._type = TokenType::T_EOF; //�������� EOF-�����
		return t;
	}

	return m_tokenList[m_currentTokenIndex++];
}

TokenType Parser::currentType() { //��� �������� ������
	return getCurrent().GetType();
}

void Parser::printTree(TreeNode* node) {
	std::cout << std::endl << std::endl;
	if (node == NULL)
		if (rootNode != NULL)
			node = rootNode;
		else
			std::cout << "ooops!" << std::endl;

	node->printNode(0);
	std::cout << std::endl;
	std::cout << std::endl;
}

void Parser::parse() { //�������
	rootNode = new TreeNode(NodeType::NODE_COMPILATION_UNIT); //������� ���� COMPILATION_UNIT

	while (currentType() != TokenType::T_EOF) { //���� �� EOF-�����
		rootNode->addChild(parse_compilation_unit()); //�������� ���� ������� � �� �������, ���������� ���������� (���� FUNCTIONDECL) � �������� ������� COMPILATION_UNIT
	}
}

TreeNode* Parser::parse_compilation_unit() { //�������� ���� ������� � �� �������
	if (currentType() == T_VOID || currentType() == T_INT || currentType() == T_CHAR || currentType() == T_IDENTIFIER) {
		return parse_function_declaration();
	}

	return nullptr;
}

TreeNode* Parser::parse_function_declaration() { //������� �������
	TreeNode* node = new TreeNode(NodeType::NODE_FUNCTIONDECL); //������� ���� FUNCTIONDECL
	node->addChild(parse_function_header()); //�������� ����� ������������� ������� (���, �������������, ���������) � �������� ������� � FUNCTIONDECL

	if (currentType() == TokenType::T_SEMICOLON) { //���� ������� �����- ����� � �������
		getNext(); //������� ��������� �����
	}
	else if (currentType() == TokenType::T_LBRACE) { //���� ������� �����- ����������� �������� ������
		node->addChild(parse_block()); //�������� � ���� FUNCTIONDECL ���� BLOCK � ��������� ����� (����������� �������� ������)
	}
	else { //�����- ������
		std::cout << "Expection function definition block after function header!" << std::endl;
	}

	return node;
}


TreeNode* Parser::parse_function_header() { //����� ������������� ������� (���, �������������, ���������)
	TreeNode* functionHeaderNode = new TreeNode(NodeType::NODE_FUNCTIONHEADER); //������� ���� FUNCTIONHEAD
	functionHeaderNode->addChild(parse_function_returntype()); //�������� � ����� ���� ���� RETURNTYPE � ��������, ���������� ��� �������

	if (currentType() == TokenType::T_IDENTIFIER) { //���� ������� �����- ������������� �������
		functionHeaderNode->addChild(new TreeNode(NodeType::NODE_IDENTIFIER, getCurrent())); //������� ���� �� ������-�������������� ������� � �������� ��� � �������� �������

		getNext(); //������� ��������� �����

		if (getCurrent().GetType() == TokenType::T_LPAREN) { //���� ��� ������������� ������
			getNext(); //������� ��������� �����
			functionHeaderNode->addChild(parse_function_param_list()); //������� ���� PARAMLIST � ��� ������� (���� �� ���������� �������) � �������� ��� � �������� �������
		}
		else {
			std::cout << "Expecting '(' after function identifier." << std::endl;
		}
	}
	else { //���� ������� �����- �� ������������� �������, �� ������
		std::cout << "Expecting function identifier." << std::endl;
	}

	return functionHeaderNode;
}

TreeNode* Parser::parse_function_returntype() { //������� ��� ������� � ���� ���� (����������� ���� ���), ����������� �������� ���������� ���� RETURNTYPE (������������ ��������� ����)
	TreeNode* node = new TreeNode(NodeType::NODE_RETURNTYPE); //������� ���� RETURNTYPE
	node->addChild(new TreeNode(getNodeFromToken(getCurrent()))); //�������� � ���� RETURNTYPE ����, � ����� �������� ������, � ���� ������� 
	getNext(); //������� ��������� �����

	return node;
}

TreeNode* Parser::parse_function_args() {
	TreeNode* node;

	getNext();
	node = new TreeNode(NodeType::NODE_FUNCARGS);

	bool _loop = false;

	if (currentType() != TokenType::T_RPAREN) {
		do {
			TreeNode* nodeExpr = parse_expression();
			if (nodeExpr->m_nodeType == NodeType::NODE_FACTOR)
				nodeExpr = nodeExpr->getChild(0);

			node->addChild(nodeExpr);

			_loop = (currentType() == TokenType::T_COMMA);

			if (_loop)
				getNext();
		} while (_loop);
	}

	if (currentType() != TokenType::T_RPAREN) {
		std::cout << "Error: Expecting ')' after argument list!" << std::endl;
	}

	getNext();

	return node;
}

TreeNode* Parser::parse_param() { //������� ���������
	TreeNode* node = new TreeNode(NodeType::NODE_PARAM); //������� ���� PARAM

	if (currentType() == T_INT || currentType() == T_CHAR) { //���� ��� ���������� int ��� char
		node->addChild(new TreeNode(getNodeFromToken(getCurrent()))); //�������� ��� ��������� � �������� ������� PARAM
	}
	else //����� (�� int ��� char) ������
		std::cout << "Expecting valid data type in parameter list" << std::endl;


	getNext(); //������� ��������� �����
	//TreeNode* nodeReference = parse_reference();
	//getNext();

	if (currentType() != T_IDENTIFIER) { //���� ����� ���� ��������� �� ������� ������������� ���������
		std::cout << "Expecting identifier in param list" << std::endl; //������
		exit(0);
	}
	//���� ����� ���� ��������� ������� ������������� ���������
	node->addChild(new TreeNode(NodeType::NODE_IDENTIFIER, getCurrent())); //�������� ������������� ��������� � �������� ������� PARAM
	//ast->AddChild(node, nodeReference);
	getNext(); //������� ��������� �����
	//ast->AddChild(node, parse_dimension_block());

	return node;
}

TreeNode* Parser::parse_function_param_list() {
	TreeNode* node = new TreeNode(NodeType::NODE_PARAMLIST); //������� ���� PARAMLIST

	if (currentType() == TokenType::T_VOID || currentType() == TokenType::T_RPAREN) { //���� � ������� ��� ����������
		node->addChild(new TreeNode(NodeType::NODE_VOID)); //�������� � ���� PARAMLIST ���� VOID � ���� �������
		getNext(); //������� ��������� �����
		return node;
	}
	//����� (���� � ������� ���� ���������)
	TokenType tokenType = TokenType::T_UNKNOWN;
	bool loop = true;

	while (loop) {
		tokenType = currentType();

		node->addChild(parse_param()); //�������� � ���� PARAMLIST ��������� � ���� ��������
		loop = (currentType() == (T_COMMA));
		getNext();
	}

	return node;
}

TreeNode* Parser::parse_block() { //������� ����� (����������� �������� ������)
	TreeNode* node = new TreeNode(NodeType::NODE_BLOCK); //������� ���� BLOCK

	//getNext();

	if (currentType() == TokenType::T_LBRACE) { //���� ������� �����- ����������� �������� ������
		getNext(); //������� ��������� �����

		while (currentType() != TokenType::T_RBRACE) { //���� �� ����� �� ����������� �������� ������
			if (currentType() == TokenType::T_ASM) { //���� �����- ���������
				node->addChild(parse_assembly()); //������� ����������
			}
			else {
				node->addChild(parse_statement()); //�������� ���� STATEMENT (� ������ ��������� ����� �������) � �������� ������� ���� BLOCK
			}
		}
	}
	else { //���� ������� �����- �� ����������� �������� ������
		std::cout << "A code block has to start with '{'." << std::endl;
	}

	getNext(); //������� ��������� �����

	return node;
}


TreeNode* Parser::parse_assembly() { //������� ����������
	TreeNode* node = new TreeNode(NodeType::NODE_ASSEMBLY);
	TreeNode* result = NULL;

	getNext();
	if (currentType() == TokenType::T_LBRACE) {
		while (currentType() != TokenType::T_RBRACE) {
			getNext();
			if (currentType() == TokenType::T_STRING)
				node->addChild(new TreeNode(NodeType::NODE_ASSEMBLY_LINE, getCurrent()));
		}
	}
	else {
		std::cout << "Error: Inline assembly block must begin with '{'!" << std::endl;
	}

	getNext();

	return node;
}


TreeNode* Parser::parse_statement() { //������ ������� ���� � ����� �������
	TreeNode* node = new TreeNode(NodeType::NODE_STATEMENT); //������� ���� STATEMENT

	/*if (currentType() == TokenType::T_IDENTIFIER)
	{
	}
	else */
	if (currentType() == TokenType::T_SEMICOLON) { //���� ������� �����- ����� � �������
		getNext(); //������� ��������� �����
		return nullptr;
	}
	/*else if (currentType() == TokenType::T_WHILE) { //���� ������� �����- WHILE (����) !!!������!!!

	}*/
	else if (currentType() == TokenType::T_INT) { //���� ������� �����- int
		node->addChild(parse_declaration_statement()); //�������� ���� DECLBLOCK (� ��������� ���� � ����� �������) � �������� ������� ���� STATEMENT
	}
	else if (currentType() == TokenType::T_PRINT) {
		node->addChild(parse_print());
	}
	else if (currentType() == TokenType::T_IF) {
		node->addChild(parse_if());
	}
	else {
		node->addChild(parse_expression());
	}

	return node;
}

TreeNode* Parser::parse_if() {
	TreeNode* node = new TreeNode(NodeType::NODE_IF);
	getNext();
	getNext();

	if (currentType() == TokenType::T_IDENTIFIER || currentType() == TokenType::T_DIGIT) {
		node->addChild(parse_expression());
	}

	getNext();

	if (currentType() == TokenType::T_LBRACE) {
		//getNext();
		node->addChild(parse_block());
	}
	else if (currentType() == TokenType::T_IDENTIFIER) {
		getNext();
		node->addChild(parse_statement());
	}

	if (currentType() == TokenType::T_ELSE) {
		node->addChild(parse_else());
	}
	return node;
}

TreeNode* Parser::parse_else() {
	TreeNode* node = new TreeNode(NodeType::NODE_ELSE);
	getNext();
	if (currentType() == TokenType::T_LBRACE) {
		//getNext();
		node->addChild(parse_block());
	}
	else if (currentType() == TokenType::T_IDENTIFIER) {
		//getNext();
		node->addChild(parse_statement());
	}
	return node;
}

TreeNode* Parser::parse_print() {
	TreeNode* node = new TreeNode(NodeType::NODE_PRINT);

	getNext();
	getNext();

	if (currentType() == TokenType::T_STRING) {
		node->addChild(new TreeNode(NodeType::NODE_STRING, getCurrent()));
	}
	else if (currentType() == TokenType::T_IDENTIFIER || currentType() == TokenType::T_DIGIT) {
		node->addChild(parse_expression());
	}
	
	getNext();
	return node;
}

TreeNode* Parser::parse_declaration_statement() { //������� ���� � ����� �������
	TreeNode* node = new TreeNode(NodeType::NODE_DECLARATION); //������ ���� DECLBLOCK

	node->addChild(new TreeNode(getNodeFromToken(getCurrent()))); //�������� � ���� DECLARATION ����, � ����� �������� ������, � ���� �������

	getNext(); //������� ��������� �����

	if (currentType() == TokenType::T_IDENTIFIER) { //���� ������� �����- �������������
		node->addChild(new TreeNode(NodeType::NODE_IDENTIFIER, getCurrent())); //�������� � ���� DECLARATION ����-������������� � value = ��� �������������� � �������� �������
		getNext(); //������� ��������� �����
		node->addChild(parse_initializer()); //�������� � ���� DECLARATION ���� � ��������� ������������� � �������� �������
	}
	else { //���� ��������� �����- �� �������������- ������
		std::cout << "Expecting identifier after type specifier!" << std::endl;
	}

	return node;
}

TreeNode* Parser::parse_expression() { //������� ���������
	TreeNode* node = parse_expression_relation(); //����� ������ ������� ��������� (� ������ ���������)
	TreeNode* opNode = nullptr;

	if (currentType() != TokenType::T_OP_ASSIGN) { //���� ������� �����- �� ������������ ("=")
		//getNext();
		return node;
	}
	else { //���� ������� �����- ������������ ("=")
		opNode = new TreeNode(getNodeFromToken(getCurrent())); //������� ���� � ����� �������� ������ ("=" ASSIGN)

		getNext(); //������� ��������� �����

		opNode->addChild(node); //�������� � ���� ������������ (ASSIGN) ��������� � �������� �������
		opNode->addChild(parse_expression()); //�������� � ���� ������������ (ASSIGN) ����������� ����� �������� ��������� � �������� �������

		return opNode;
	}

	return opNode;
}

TreeNode* Parser::parse_initializer() { //������� �������������
	TreeNode* node = nullptr;

	if (currentType() == TokenType::T_OP_ASSIGN) { //���� ������� �����- ������������ ("=")
		TreeNode* expressionNode = parse_expression(); //������� ���� � ��������� � ���� ������� ���������

		if (expressionNode != nullptr) { //���� ��������� �������
			node = new TreeNode(NodeType::NODE_INITIALIZER); //������� ���� INITIALIZER
			node->addChild(expressionNode);//�������� � ���� ���� ��������� � �������� �������
		}
	}

	return node;
}

TreeNode* Parser::parse_expression_relation() { //����� ������ ������� ��������� (� ������ ���������)
	TreeNode* node = parse_expression_addition(); //������� ���� � ��������� � ���� ������ ������� ���������
	TreeNode* opNode = nullptr;

	while (currentType() == TokenType::T_OP_GREATER ||
		currentType() == TokenType::T_OP_GREATEREQUAL ||
		currentType() == TokenType::T_OP_LESS ||
		currentType() == TokenType::T_OP_LESSEQUAL) { //���� �����- ">",">=","<","<="
		opNode = new TreeNode(getNodeFromToken(getCurrent())); //������� ���� � ����� ������
		opNode->addChild(node); //�������� � ���������� ���� (� ����� ���������) ������� � ���� ��������� (������, ������������ ����� ���������)

		getNext(); //������� ��������� �����

		opNode->addChild(parse_expression_addition()); //�������� � ���������� ���� (� ����� ���������) ������� � ���� ��������� (�������, ������������ ����� ���������)
		node = opNode; //������� ��� ��������� � node
	}

	return node;
}

TreeNode* Parser::parse_expression_addition() { //������ ������� ���������
	TreeNode* node;
	TreeNode* opNode;

	node = parse_expression_multiplication(); //������� ���� � ��������� � ���� ������� ���������

	while (currentType() == TokenType::T_OP_BINARY_ADD || currentType() == TokenType::T_OP_BINARY_SUBSTRACT) { //���� ������� �����- �������� ��� ���������
		opNode = new TreeNode(getNodeFromToken(getCurrent()), getCurrent()); //������� ���� � ����� � ��������� ������
		opNode->addChild(node); //�������� � ���������� ���� ������� ��������� � ���� �������

		getNext(); //������� ��������� �����
		opNode->addChild(parse_expression_multiplication()); //��������� ������� ��������� � �������� ���������� ����� � �������� �������
		node = opNode; //������� ��� ��������� � node
	}

	return node;
}

TreeNode* Parser::parse_expression_multiplication() { //������� ���������
	TreeNode* child;
	TreeNode* op;

	child = parse_unary2(); //������� �������� ���������

	while (currentType() == TokenType::T_STAR || currentType() == TokenType::T_OP_DIVIDE || currentType() == TokenType::T_MOD) { //���� �����- ���������, ������������� ������� ��� ������ ������� 
		op = new TreeNode(getNodeFromToken(getCurrent()), getCurrent()); //������� ���� � ����� � ��������� ������
		op->addChild(child); //�������� � ���������� ���� ������� �������� ��������� � ���� �������

		getNext(); //������� ��������� �����

		op->addChild(parse_unary2()); //��������� ������� �������� ��������� � �������� ���������� ����� � �������� �������
		child = op; //������� ��� ��������� � child
	}

	return child;
}

TreeNode* Parser::parse_unary1() { //������� ����������/�������/�����
	TreeNode* node;
	TreeNode* child;

	node = parse_factor(); //������� �������������� (����������, �������) ��� �����

	while (currentType() == TokenType::T_LPAREN || currentType() == TokenType::T_LBRACKET) { //���� �����- ������������� ������� ��� ���������� ������
		if (currentType() == T_LPAREN) { //���� �����- ������������� ������� ������
			child = node;
			node = new TreeNode(NodeType::NODE_FUNCCALL); //������� ���� FUNCCALL
			node->addChild(child->getChild(0)); //�������� � ���� ������������ ������� FUNCCALL

			if (currentType() != TokenType::T_RPAREN) { //���� �����- ������������� ������� ������
				TreeNode* nodeArgs = parse_function_args(); //������� ���� �� �������� ���������� ���������� �������
				if (nodeArgs->m_children.size() > 0) //���� ���� ���������
					node->addChild(nodeArgs); //�������� �� � �������� ������� � ���� FUNCCALL
			}
		}

		if (currentType() == TokenType::T_LBRACKET) { //���� �����- ������������� ���������� ������
			getNext(); //������� ��������� �����

			child = node;
			node = new TreeNode(NodeType::NODE_INDEXER); //������� ���� INDEXER
			node->addChild(child); //�������� � ���� INDEXER ���� � ��������� �������������� � �������� �������
			node->addChild(parse_expression());

			if (currentType() != TokenType::T_RBRACKET) { //���� �����- �� ������������� ���������� ������
				std::cout << "Error: Expecting ']' after array index expression." << std::endl;
				exit(0);
			}
			getNext(); //������� ��������� �����
		}
	}

	return node;
}

TreeNode* Parser::parse_unary2() { //������� �������� ���������

	TreeNode* node = nullptr; // = parse_unary1();

	if (currentType() == TokenType::T_OP_BINARY_ADD || currentType() == TokenType::T_OP_BINARY_SUBSTRACT) { //���� ������� �����- �������� ��� ���������
		switch (currentType()) {
		case T_OP_BINARY_ADD: //��������
			node = new TreeNode(NodeType::NODE_UNARY_ADD); //������� ���� �������� ADD
			break;
		case T_OP_BINARY_SUBSTRACT: //���������
			node = new TreeNode(NodeType::NODE_UNARY_SUBSTRACT); //������� ���� ��������� SUBSTRACT
			break;
		}

		getNext(); //������� ��������� �����
		node->addChild(parse_unary2()); //�������� � ���������� ���� ������� (����������/�������/�����) ����� ����������� ����� �������� �������� ���������
	}
	else
		node = parse_unary1(); //������� ����������/�������/�����

	return node;
}

TreeNode* Parser::parse_factor() { //������� �������������� (����������, �������) ��� �����
	TreeNode* node = new TreeNode(NodeType::NODE_FACTOR); //������� ���� FACTOR

	if (currentType() == TokenType::T_LPAREN) { //���� �����- ������������� ������� ������
		getNext(); //������� ��������� �����
		node->addChild(parse_expression());
		getNext(); //������� ��������� �����
	}
	else if (currentType() == TokenType::T_IDENTIFIER) { //���� �����- �������������
		node->addChild(new TreeNode(NodeType::NODE_IDENTIFIER, getCurrent())); //�������� � ���� FACTOR ���� IDENTIFIER � value ������
		getNext(); //������� ��������� �����
	}
	else if (currentType() == TokenType::T_DIGIT ||
		currentType() == TokenType::T_STRING ||
		currentType() == TokenType::T_CHAR) { //���� �����- �����, ������ ��� ������
		switch (currentType()) {
		case TokenType::T_DIGIT: //���� �����- �����
			node->addChild(new TreeNode(NodeType::NODE_DIGIT, getCurrent())); //�������� � ���� FACTOR ���� DIGIT � value ������
			break;
			//!!!�������� ��� char!!!
		}

		getNext(); //������� ��������� �����
	}

	if (node->m_children.size() <= 0) //���� � ���� FACTOR �� ��������� ��������
		node = nullptr;

	return node;
}