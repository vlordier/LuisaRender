//
// Integration tests for the scene description language (SDL) tag layer.
// Tests the parse_scene_node_tag / scene_node_tag_description roundtrip and
// the SceneDesc node-definition API – all exercised purely on the CPU without
// any GPU backend.
//

#include <cassert>
#include <cstdio>
#include <string_view>

#include <sdl/scene_desc.h>
#include <sdl/scene_node_tag.h>

using namespace luisa::render;
using namespace std::string_view_literals;

namespace {

// ── tag parsing ────────────────────────────────────────────────────────────

void test_tag_parse_canonical_names() noexcept {
    assert(parse_scene_node_tag("Camera"sv) == SceneNodeTag::CAMERA);
    assert(parse_scene_node_tag("Shape"sv) == SceneNodeTag::SHAPE);
    assert(parse_scene_node_tag("Surface"sv) == SceneNodeTag::SURFACE);
    assert(parse_scene_node_tag("Light"sv) == SceneNodeTag::LIGHT);
    assert(parse_scene_node_tag("Film"sv) == SceneNodeTag::FILM);
    assert(parse_scene_node_tag("Filter"sv) == SceneNodeTag::FILTER);
    assert(parse_scene_node_tag("Sampler"sv) == SceneNodeTag::SAMPLER);
    assert(parse_scene_node_tag("Integrator"sv) == SceneNodeTag::INTEGRATOR);
    assert(parse_scene_node_tag("Environment"sv) == SceneNodeTag::ENVIRONMENT);
    assert(parse_scene_node_tag("Texture"sv) == SceneNodeTag::TEXTURE);
    assert(parse_scene_node_tag("Spectrum"sv) == SceneNodeTag::SPECTRUM);
    assert(parse_scene_node_tag("Medium"sv) == SceneNodeTag::MEDIUM);
    assert(parse_scene_node_tag("PhaseFunction"sv) == SceneNodeTag::PHASE_FUNCTION);
}

void test_tag_parse_aliases() noexcept {
    // Aliases that map to the same tag.
    assert(parse_scene_node_tag("cam"sv) == SceneNodeTag::CAMERA);
    assert(parse_scene_node_tag("object"sv) == SceneNodeTag::SHAPE);
    assert(parse_scene_node_tag("obj"sv) == SceneNodeTag::SHAPE);
    assert(parse_scene_node_tag("surf"sv) == SceneNodeTag::SURFACE);
    assert(parse_scene_node_tag("lightsource"sv) == SceneNodeTag::LIGHT);
    assert(parse_scene_node_tag("illuminant"sv) == SceneNodeTag::LIGHT);
    assert(parse_scene_node_tag("illum"sv) == SceneNodeTag::LIGHT);
    assert(parse_scene_node_tag("xform"sv) == SceneNodeTag::TRANSFORM);
    assert(parse_scene_node_tag("lightsampler"sv) == SceneNodeTag::LIGHT_SAMPLER);
    assert(parse_scene_node_tag("env"sv) == SceneNodeTag::ENVIRONMENT);
    assert(parse_scene_node_tag("tex"sv) == SceneNodeTag::TEXTURE);
    assert(parse_scene_node_tag("texmapping"sv) == SceneNodeTag::TEXTURE_MAPPING);
    assert(parse_scene_node_tag("spec"sv) == SceneNodeTag::SPECTRUM);
    assert(parse_scene_node_tag("generic"sv) == SceneNodeTag::DECLARATION);
    assert(parse_scene_node_tag("template"sv) == SceneNodeTag::DECLARATION);
}

void test_tag_parse_case_insensitive() noexcept {
    // The parser normalises to lower-case before lookup.
    assert(parse_scene_node_tag("CAMERA"sv) == SceneNodeTag::CAMERA);
    assert(parse_scene_node_tag("Camera"sv) == SceneNodeTag::CAMERA);
    assert(parse_scene_node_tag("cAmErA"sv) == SceneNodeTag::CAMERA);
    assert(parse_scene_node_tag("SHAPE"sv) == SceneNodeTag::SHAPE);
    assert(parse_scene_node_tag("FILM"sv) == SceneNodeTag::FILM);
}

void test_tag_parse_unknown_returns_root() noexcept {
    // An unrecognised tag description falls back to ROOT.
    assert(parse_scene_node_tag("ThisDoesNotExist"sv) == SceneNodeTag::ROOT);
    assert(parse_scene_node_tag(""sv) == SceneNodeTag::ROOT);
}

// ── tag description ────────────────────────────────────────────────────────

void test_tag_description_roundtrip() noexcept {
    // scene_node_tag_description must return a non-empty string for every
    // well-defined tag except ROOT/INTERNAL/DECLARATION (which are internal).
    const SceneNodeTag public_tags[] = {
        SceneNodeTag::CAMERA,
        SceneNodeTag::SHAPE,
        SceneNodeTag::SURFACE,
        SceneNodeTag::LIGHT,
        SceneNodeTag::TRANSFORM,
        SceneNodeTag::FILM,
        SceneNodeTag::FILTER,
        SceneNodeTag::SAMPLER,
        SceneNodeTag::INTEGRATOR,
        SceneNodeTag::LIGHT_SAMPLER,
        SceneNodeTag::ENVIRONMENT,
        SceneNodeTag::TEXTURE,
        SceneNodeTag::TEXTURE_MAPPING,
        SceneNodeTag::SPECTRUM,
        SceneNodeTag::MEDIUM,
        SceneNodeTag::PHASE_FUNCTION,
    };
    for (auto tag : public_tags) {
        auto desc = scene_node_tag_description(tag);
        assert(!desc.empty());
        // The description must not start with "__" (reserved for internal tags).
        assert(desc.substr(0, 2) != "__");
    }
}

// ── SceneDesc programmatic construction ───────────────────────────────────

void test_scene_desc_construction() noexcept {
    SceneDesc desc;

    // The root node is always present and has the correct identifier.
    assert(desc.root() != nullptr);
    assert(desc.root()->identifier() == SceneDesc::root_node_identifier);

    // Initially the global node set is empty (root is not in the set).
    assert(desc.nodes().empty());
}

void test_scene_desc_define_node() noexcept {
    SceneDesc desc;

    // Define a Camera node named "main_camera".
    auto *cam = desc.define("main_camera"sv, SceneNodeTag::CAMERA, "pinhole"sv);
    assert(cam != nullptr);
    assert(cam->identifier() == "main_camera"sv);
    assert(cam->tag() == SceneNodeTag::CAMERA);
    assert(cam->impl_type() == "pinhole"sv);

    // The node must now appear in the global set.
    assert(desc.nodes().size() == 1u);

    // Define a second node.
    auto *film = desc.define("hdr_film"sv, SceneNodeTag::FILM, "color"sv);
    assert(film != nullptr);
    assert(desc.nodes().size() == 2u);
}

void test_scene_desc_reference() noexcept {
    SceneDesc desc;

    // Referencing a node before it is defined creates a forward-reference stub.
    auto *ref = desc.reference("future_node"sv);
    assert(ref != nullptr);
    assert(ref->identifier() == "future_node"sv);

    // Defining it later fills in the stub.
    auto *real = desc.define("future_node"sv, SceneNodeTag::TEXTURE, "constant"sv);
    assert(real != nullptr);
    // Both pointers must refer to the same node in the set.
    auto found = desc.node("future_node"sv);
    assert(found != nullptr);
    assert(found->identifier() == "future_node"sv);
}

}// namespace

int main() {
    test_tag_parse_canonical_names();
    test_tag_parse_aliases();
    test_tag_parse_case_insensitive();
    test_tag_parse_unknown_returns_root();
    test_tag_description_roundtrip();
    test_scene_desc_construction();
    test_scene_desc_define_node();
    test_scene_desc_reference();
    std::puts("All scene node tag / SceneDesc integration tests passed.");
    return 0;
}
