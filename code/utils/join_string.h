#pragma once

namespace util {

template <typename TIter>
SCP_string join_range(TIter&& begin, TIter&& end, const SCP_string& joiner)
{
	SCP_stringstream stream;
	bool first = true;

	for (auto iter = begin; iter != end; ++iter) {
		stream << *iter;
		if (first) {
			first = false;
		} else {
			stream << joiner;
		}
	}

	return stream.str();
}

template <typename TContainer>
SCP_string join_container(TContainer& container, const SCP_string& joiner)
{
	return join_range(std::begin(container), std::end(container), joiner);
}

} // namespace util
