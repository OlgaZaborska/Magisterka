#include "learning/DiscriminationTree.h"

#include "learning/Discriminator.h"

#include <optional>
#include <stdexcept>
#include <unordered_map>

struct DiscriminationTree::Node {
    std::optional<AccessSequence> accessSequence;
    std::unique_ptr<Discriminator> discriminator;
    std::unordered_map<Weight, std::unique_ptr<Node>> children;

    explicit Node(const AccessSequence& sequence)
        : accessSequence(sequence) {
    }

    Node(const Word& suffix, ResidualFunction& residualFunction)
        : discriminator(
            std::make_unique<Discriminator>(
                suffix,
                residualFunction
            )
        ) {
    }

    bool isLeaf() const {
        return accessSequence.has_value();
    }
};

DiscriminationTree::DiscriminationTree(
    const AccessSequence& initialAccessSequence,
    ResidualFunction& residualFunction
)
    : root_(std::make_unique<Node>(initialAccessSequence)),
      residualFunction_(residualFunction) {
}

DiscriminationTree::~DiscriminationTree() = default;

DiscriminationTree::Node* DiscriminationTree::findLeaf(
    Node* node,
    const AccessSequence& sequence
) {
    if (node->isLeaf()) {
        if (node->accessSequence.value() == sequence) {
            return node;
        }

        return nullptr;
    }

    for (auto& [weight, child] : node->children) {
        Node* result = findLeaf(child.get(), sequence);

        if (result != nullptr) {
            return result;
        }
    }

    return nullptr;
}

AccessSequence DiscriminationTree::sift(const Word& word) {
    Node* current = root_.get();

    while (!current->isLeaf()) {
        Weight result =
            current->discriminator->evaluate(word);

        auto it = current->children.find(result);

        if (it == current->children.end()) {
            throw std::runtime_error(
                "No matching branch in discrimination tree"
            );
        }

        current = it->second.get();
    }

    return current->accessSequence.value();
}

void DiscriminationTree::split(
    const AccessSequence& oldAccessSequence,
    const AccessSequence& newAccessSequence,
    const Word& discriminatorSuffix
) {
    if (oldAccessSequence == newAccessSequence) {
        throw std::invalid_argument(
            "Cannot split a leaf using the same access sequence"
        );
    }

    Node* leaf = findLeaf(root_.get(), oldAccessSequence);

    if (leaf == nullptr) {
        throw std::invalid_argument(
            "Access sequence not found in discrimination tree"
        );
    }

    auto discriminator = std::make_unique<Discriminator>(
        discriminatorSuffix,
        residualFunction_
    );

    Weight oldResult =
        discriminator->evaluate(oldAccessSequence.word());

    Weight newResult =
        discriminator->evaluate(newAccessSequence.word());

    if (oldResult == newResult) {
        throw std::invalid_argument(
            "Discriminator does not distinguish the access sequences"
        );
    }

    leaf->accessSequence.reset();
    leaf->discriminator = std::move(discriminator);

    leaf->children.emplace(
        oldResult,
        std::make_unique<Node>(oldAccessSequence)
    );

    leaf->children.emplace(
        newResult,
        std::make_unique<Node>(newAccessSequence)
    );
}


std::vector<AccessSequence> DiscriminationTree::accessSequences() const {
    std::vector<AccessSequence> result;
    collectAccessSequences(root_.get(), result);
    return result;
}

void DiscriminationTree::collectAccessSequences(
    const DiscriminationTree::Node* node,
    std::vector<AccessSequence>& result
) const {
    if (node->isLeaf()) {
        result.push_back(node->accessSequence.value());
        return;
    }

    for (const auto& entry : node->children) {
        collectAccessSequences(entry.second.get(), result);
    }
}
