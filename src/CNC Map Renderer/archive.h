#pragma once

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>

#include "File.h"

namespace VFS {
	class Archive : private boost::noncopyable {
	public:
		virtual ~Archive() {}

		virtual std::string path() = 0;
		virtual boost::shared_ptr<File> open(const std::string& filename, bool writable) = 0;
		virtual bool Exists(const std::string& filename) const = 0;
	};
}
