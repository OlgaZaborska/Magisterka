
#pragma once

#include "core/Types.h"
#include "learning/AccessSequence.h"
#include "learning/ResidualFunction.h"

#include <memory>

class DiscriminationTree {
private:
    struct Node;

    std::unique_ptr<Node> root_;
    ResidualFunction& residualFunction_;

    Node* findLeaf(Node* node, const AccessSequence& sequence);

    void collectAccessSequences(const Node* node, std::vector<AccessSequence>& result) const;

public:
    DiscriminationTree(
        const AccessSequence& initialAccessSequence,
        ResidualFunction& residualFunction
    );
    ~DiscriminationTree();

    void split(
        const AccessSequence& oldAccessSequence,
        const AccessSequence& newAccessSequence,
        const Word& discriminatorSuffix
    );

    AccessSequence sift(const Word& word);

    std::vector<AccessSequence> accessSequences() const;


};
