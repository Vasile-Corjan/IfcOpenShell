/********************************************************************************
 *                                                                              *
 * This file is part of IfcOpenShell.                                           *
 *                                                                              *
 * IfcOpenShell is free software: you can redistribute it and/or modify         *
 * it under the terms of the Lesser GNU General Public License as published by  *
 * the Free Software Foundation, either version 3.0 of the License, or          *
 * (at your option) any later version.                                          *
 *                                                                              *
 * IfcOpenShell is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
 * Lesser GNU General Public License for more details.                          *
 *                                                                              *
 * You should have received a copy of the Lesser GNU General Public License     *
 * along with this program. If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                              *
 ********************************************************************************/

#ifndef IFCFILE_H
#define IFCFILE_H

#include <map>
#include <set>

#include "ifc_parse_api.h"

#include "../ifcparse/IfcParse.h"
#include "../ifcparse/IfcSpfHeader.h"
#include "../ifcparse/IfcSchema.h"

namespace IfcParse {

/// This class provides several static convenience functions and variables
/// and provide access to the entities in an IFC file
class IFC_PARSE_API IfcFile {
public:
	typedef std::map<const IfcParse::declaration*, IfcEntityList::ptr> entities_by_type_t;
	typedef std::map<unsigned int, IfcUtil::IfcBaseClass*> entity_by_id_t;
	typedef std::map<std::string, IfcUtil::IfcBaseClass*> entity_by_guid_t;
	typedef std::map<unsigned int, std::vector<unsigned int> > entities_by_ref_t;
	typedef entity_by_id_t::const_iterator const_iterator;

	class type_iterator : public entities_by_type_t::const_iterator {
	public:
		type_iterator() : entities_by_type_t::const_iterator() {};

		type_iterator(const entities_by_type_t::const_iterator& it)
			: entities_by_type_t::const_iterator(it)
		{};

		entities_by_type_t::key_type const * operator->() const {
			return &entities_by_type_t::const_iterator::operator->()->first;
		}

		const entities_by_type_t::key_type& operator*() const {
			return entities_by_type_t::const_iterator::operator*().first;
		}

		const std::string& as_string() const {
			return (**this)->name();
		}
	};

private:
	typedef std::map<IfcUtil::IfcBaseClass*, IfcUtil::IfcBaseClass*> entity_entity_map_t;

	bool parsing_complete_, good_;

	const IfcParse::schema_definition* schema_;
	const IfcParse::declaration* ifcroot_type_;

	entity_by_id_t byid;
	entities_by_type_t bytype;
	entities_by_type_t bytype_excl;
	entities_by_ref_t byref;
	entity_by_guid_t byguid;
	entity_entity_map_t entity_file_map;

	unsigned int MaxId;

	IfcSpfHeader _header;

	void setDefaultHeaderValues();

	void initialize_(IfcParse::IfcSpfStream* f);

public:
	IfcParse::IfcSpfLexer* tokens;
	IfcParse::IfcSpfStream* stream;
	
#ifdef USE_MMAP
	IfcFile(const std::string& fn, bool mmap = false);
#else
	IfcFile(const std::string& fn);
#endif
	IfcFile(std::istream& fn, int len);
	IfcFile(void* data, int len);
	IfcFile(IfcParse::IfcSpfStream* f);
	IfcFile(const IfcParse::schema_definition* schema = IfcParse::schema_by_name("IFC4"));

	virtual ~IfcFile();

	bool good() const { return good_; }
	
	/// Returns the first entity in the file, this probably is the entity
	/// with the lowest id (EXPRESS ENTITY_INSTANCE_NAME)
	const_iterator begin() const;
	/// Returns the last entity in the file, this probably is the entity
	/// with the highest id (EXPRESS ENTITY_INSTANCE_NAME)
	const_iterator end() const;

	type_iterator types_begin() const;
	type_iterator types_end() const;

	type_iterator types_incl_super_begin() const;
	type_iterator types_incl_super_end() const;

	/// Returns all entities in the file that match the template argument.
	/// NOTE: This also returns subtypes of the requested type, for example:
	/// IfcWall will also return IfcWallStandardCase entities
	template <class T>
	typename T::list::ptr instances_by_type() {
		IfcEntityList::ptr untyped_list = instances_by_type(&T::Class());
		if (untyped_list) {
			return untyped_list->as<T>();
		} else {
			return typename T::list::ptr(new typename T::list);
		}
	}

	template <class T>
	typename T::list::ptr instances_by_type_excl_subtypes() {
		IfcEntityList::ptr untyped_list = instances_by_type_excl_subtypes(&T::Class());
		if (untyped_list) {
			return untyped_list->as<T>();
		} else {
			return typename T::list::ptr(new typename T::list);
		}
	}

	/// Returns all entities in the file that match the positional argument.
	/// NOTE: This also returns subtypes of the requested type, for example:
	/// IfcWall will also return IfcWallStandardCase entities
	IfcEntityList::ptr instances_by_type(const IfcParse::declaration*);

	/// Returns all entities in the file that match the positional argument.
	IfcEntityList::ptr instances_by_type_excl_subtypes(const IfcParse::declaration*);

	/// Returns all entities in the file that match the positional argument.
	/// NOTE: This also returns subtypes of the requested type, for example:
	/// IfcWall will also return IfcWallStandardCase entities
	IfcEntityList::ptr instances_by_type(const std::string& t);

	/// Returns all entities in the file that reference the id
	IfcEntityList::ptr instances_by_reference(int id);

	/// Returns the entity with the specified id
	IfcUtil::IfcBaseClass* instance_by_id(int id);

	/// Returns the entity with the specified GlobalId
	virtual IfcUtil::IfcBaseClass* instance_by_guid(const std::string& guid);

	/// Performs a depth-first traversal, returning all entity instance
	/// attributes as a flat list. NB: includes the root instance specified
	/// in the first function argument.
	IfcEntityList::ptr traverse(IfcUtil::IfcBaseClass* instance, int max_level=-1);

	IfcEntityList::ptr getInverse(int instance_id, const IfcParse::declaration* type, int attribute_index);

	unsigned int FreshId() { return ++MaxId; }

	IfcUtil::IfcBaseClass* addEntity(IfcUtil::IfcBaseClass* entity);
	void addEntities(IfcEntityList::ptr es);

	void removeEntity(IfcUtil::IfcBaseClass* entity);

	const IfcSpfHeader& header() const { return _header; }
	IfcSpfHeader& header() { return _header; }

	std::string createTimestamp() const;

	void load(const IfcEntityInstanceData&);
	size_t load(unsigned entity_instance_name, Argument**& attributes, size_t num_attributes);

	void register_inverse(unsigned, Token);
	void register_inverse(unsigned, IfcUtil::IfcBaseClass*);
	void unregister_inverse(unsigned, IfcUtil::IfcBaseClass*);
    
	const IfcParse::schema_definition* schema() const { return schema_; }

	std::pair<IfcUtil::IfcBaseClass*, double> getUnit(const std::string& unit_type);
};

template <typename Schema>
class IFC_PARSE_API IfcFileWithSchema : public IfcFile {
public:
	std::pair<typename Schema::IfcNamedUnit*, double> getUnit(typename Schema::IfcUnitEnum::Value unit_type) {
		std::pair<IfcUtil::IfcBaseClass*, double> unit_info = IfcFile::getUnit(Schema::IfcUnitEnum::ToString(unit_type));
		return std::make_pair(unit_info.first->template as<typename Schema::IfcNamedUnit>(), unit_info.second);
	}
	
	/// Returns the entity with the specified GlobalId
	virtual typename Schema::IfcRoot* instance_by_guid(const std::string& guid) {
		return IfcFile::instance_by_guid(guid)->template as<typename Schema::IfcRoot>();
	}
};

}

#endif
