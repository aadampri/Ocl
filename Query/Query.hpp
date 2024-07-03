#pragma once

namespace Ocl{
    template<template<auto>typename Q, auto query>
    constexpr auto Query(Q<query> const&){return query;};
}