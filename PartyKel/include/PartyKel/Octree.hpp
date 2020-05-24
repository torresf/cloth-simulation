#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include <string>
#include <iostream>

namespace PartyKel {

    template <typename T>
    class Octree {
    private:
        /**
         * Depth of the octree.
         * depth = 0 -> 1 voxel
         * depth = 1 -> 8 voxels
         * depth = n -> 8^n voxels
         */
        int _depth;

        /**
         * Space position of the octree.
         */
        glm::vec3 _position;

        /**
         * Dimension ( = scale) of the octree.
         * Example for a dim 2 octree
         * Dimension at depth 0 : 2
         * Dimension at depth 1 : 2/2 = 1
         * Dimension at depth 2 : 2/4 = 0.5
         * Dimension at depth n : 2/2^n
         */
        glm::vec3 _dimension;

        /**
         * Children of the current octree
         * if _initChildren = false, this vector is empty
         */
        std::vector<Octree> _children;

        /**
         * La madre of the octree.
         * = nullptr if octree is root
         */
        Octree* _madre;

        /**
         * If depth != 0 this vector is empty
         */
        std::vector<T> _values;

        /**
         *  _initChildren = true if children have been constructed
         */
        bool _initChildren;

        /**
         * Initialize the 8 children of the octree.
         * Calls the constructor 8 times
         */
        void initChildren();

        /**
         * From an octree, jump recursively to madre & remove all empty children
         */
        void cleanRecursive();

    public:
        /**
         * This method only initialize depth, position & dimension of the octree
         * It does not init all children voxels
         */
        Octree(int depth, const glm::vec3 &position, const glm::vec3& dim, Octree<T>* madre = nullptr);

        /**
         * Add a value in the octree.
         * In the current implementation, the value is added at the lowest depth, i.e a leaf
         */
        void add(const T& value, const glm::vec3& position);

        /**
         * Remove a all value occurences at a specified position, if it exists
         * value should overload "==" operator
         */
        void remove(const T& value, const glm::vec3& position);

        /**
         * Returns all the values stored at a specified position in the octree
         */
        std::vector<T>& get(const glm::vec3& position);

        /**
         * Return true if the given position is inside the voxel
         */
        bool contains(const glm::vec3& position);

        void printRecursive();

    };

    // ***********************************************************************************************************************************************
    // ************************************************************** CLASS DECLARATION **************************************************************
    // ***********************************************************************************************************************************************

    template <typename T>
    Octree<T>::Octree(int depth, const glm::vec3 &position, const glm::vec3& dimension, Octree<T>* madre) :
            _depth(depth),
            _position(position),
            _dimension(dimension),
            _madre(madre),
            _initChildren(false)
    { }

    template <typename T>
    void Octree<T>::initChildren() {
        if (_initChildren || _depth == 0) return;

        glm::vec3 offset = _dimension / 4.f;
        glm::vec3 childDimension = _dimension / 2.f;

        // top of the octree
        _children.push_back(Octree<T>(_depth-1, glm::vec3(_position.x + offset.x, _position.y + offset.y, _position.z + offset.z), childDimension, this));
        _children.push_back(Octree<T>(_depth-1, glm::vec3(_position.x + offset.x, _position.y + offset.y, _position.z - offset.z), childDimension, this));
        _children.push_back(Octree<T>(_depth-1, glm::vec3(_position.x - offset.x, _position.y + offset.y, _position.z + offset.z), childDimension, this));
        _children.push_back(Octree<T>(_depth-1, glm::vec3(_position.x - offset.x, _position.y + offset.y, _position.z - offset.z), childDimension, this));

        //bottom of the octree
        _children.push_back(Octree<T>(_depth-1, glm::vec3(_position.x + offset.x, _position.y - offset.y, _position.z + offset.z), childDimension, this));
        _children.push_back(Octree<T>(_depth-1, glm::vec3(_position.x + offset.x, _position.y - offset.y, _position.z - offset.z), childDimension, this));
        _children.push_back(Octree<T>(_depth-1, glm::vec3(_position.x - offset.x, _position.y - offset.y, _position.z + offset.z), childDimension, this));
        _children.push_back(Octree<T>(_depth-1, glm::vec3(_position.x - offset.x, _position.y - offset.y, _position.z - offset.z), childDimension, this));

        _initChildren = true;
    }

    template <typename T>
    void Octree<T>::cleanRecursive() {

        bool allChildrenEmpty = true;

        for (auto& child : _children) {
            if (child._initChildren) {
                allChildrenEmpty = false;
                break;
            }
        }

        if (allChildrenEmpty) {
            _children.clear();
            _initChildren = false;
        }

        if (_madre != nullptr) {
            _madre->cleanRecursive();
            return;
        }
    }

    template <typename T>
    void Octree<T>::add(const T& value, const glm::vec3& position) {

        if (_depth == 0 && contains(position)) {
            _values.push_back(value);
            return;
        }

        if (contains(position)) {
            initChildren();
            for(auto& octree : _children) {
                if (octree.contains(position)) {
                    octree.add(value, position);
                    return;
                }
            }
        }

        std::string error = "Trying to add object at " + glm::to_string(position);
        error += " which is out of bounds of octree ( position = " + glm::to_string(_position);
        error += ", dimension = " + glm::to_string(_dimension) + " )";

        throw std::out_of_range(error);
    }

    template <typename T>
    void Octree<T>::remove(const T& value, const glm::vec3& position) {

        if (_depth == 0 && contains(position)) {
            for(int i = 0; i < _values.size(); ++i) {
                if (_values[i] == value) {
                    _values.erase(_values.begin()+i);
                    --i;
                }
            }
            if (_values.empty()) cleanRecursive();
            return;
        }

        if (contains(position)) {
            for (auto& octree : _children) {
                if (octree.contains(position)) {
                    octree.remove(value, position);
                }
            }
            return;
        }

        std::string error = "Trying to remove object at " + glm::to_string(position);
        error += " which is out of bounds of octree ( position = " + glm::to_string(_position);
        error += ", dimension = " + glm::to_string(_dimension) + " )";

        throw std::out_of_range(error);
    }

    template <typename T>
    std::vector<T>& Octree<T>::get(const glm::vec3& position) {

        if (_depth == 0 && contains(position)) {
            return _values;
        }

        if (!_initChildren) {
            return _values;
        }

        if (contains(position)) {
            for (auto& octree : _children) {
                if (octree.contains(position)) {
                    return octree.get(position);
                }
            }
        }

        std::string error = "Trying to get object at " + glm::to_string(position);
        error += " which is out of bounds of octree ( position = " + glm::to_string(_position);
        error += ", dimension = " + glm::to_string(_dimension) + " )";

        throw std::out_of_range(error);
    }

    template <typename T>
    bool Octree<T>::contains(const glm::vec3& position) {
        return !(
            position.x > _position.x + _dimension.x / 2.f ||
            position.x < _position.x - _dimension.x / 2.f ||
            position.y > _position.y + _dimension.y / 2.f ||
            position.y < _position.y - _dimension.y / 2.f ||
            position.z > _position.z + _dimension.z / 2.f ||
            position.z < _position.z - _dimension.z / 2.f
        );
    }

    template <typename T>
    void Octree<T>::printRecursive() {
        if (_initChildren) {
            for(auto& child : _children) {
                child.printRecursive();
            }
        }
    }
}