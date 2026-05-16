#pragma once

namespace ast {

class ast_node {
public:
	virtual ~ast_node() = default;
};

class expression : public ast_node {};

class statement : public ast_node {};

}