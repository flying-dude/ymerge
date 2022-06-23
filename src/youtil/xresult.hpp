#pragma once

#include <optional>
#include <string>
#include <variant>

/**
 * Implement exception-style error handling but propagate possible error conditions as part of the return
 * type instead of keeping it hidden, like C++ does with exceptions.
 */

namespace fly {

/// Used as wrapper for return values of type T but with a possible exception of type Err.
template <typename T, typename Err = std::string> struct xresult;

/// Special case of void return for xresult.
template <typename Err> struct xresult<void, Err> : std::optional<Err> {
	// https://stackoverflow.com/questions/347358/inheriting-constructors
	using std::optional<Err>::optional;
};

/**
 * Helper template struct to disambiguate xresult types in case T = Err (type aliasing). The point of this type is
 * merely to "tag" the given Err type as the failure version of xresult<T, T>.
 */
template <typename Err = std::string> struct fail {
	Err msg;
	fail(Err msg) : msg(msg) {}
};

/// Used as wrapper for return values of type T but with a possible exception of type Err.
template <typename T, typename Err> struct xresult {
	std::variant<T, Err> data;
	xresult(T &&t) : data(std::in_place_index<0>, t) {}
	xresult(fail<Err> &&e) : data(std::in_place_index<1>, e.msg) {}

	/// Check for errors.
	std::optional<Err> error() { return data.index() == 1 ? (std::optional<Err>)std::get<1>(data) : std::nullopt; }
	auto operator!() { return error(); } // syntactic sugar for error()

	/// Obtain the result value. Check for errors first using xresult::error().
	T &success() { return std::get<0>(data); }
	T &operator*() { return success(); }  // syntactic sugar for success()
	T &operator->() { return success(); } // syntactic sugar for success()
};

} // namespace fly
