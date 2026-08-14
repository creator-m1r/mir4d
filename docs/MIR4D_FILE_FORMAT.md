# MIR 4D Project Format

## Status

Version 1 of the native MIR 4D project format.

The working format is a macOS package directory with the `.mir4d` extension. The package is the native editable CAD document; a single-file archive may be added later for exchange/export.

## Package layout

```text
Example.mir4d/
├── project.mir4d.json       # required project manifest
├── Scenes/
│   └── model.mir4d.json     # required parameter/model document
├── Models/                   # imported CAD/mesh assets
├── Results/                  # simulation and calculation results
├── Documents/               # drawings, PDFs and project documents
└── Thumbnails/               # optional previews
```

## Manifest

`project.mir4d.json` identifies the project and stores lightweight session/UI state. Version 1 currently contains the existing project/session fields and `formatVersion`. New releases must preserve backward compatibility through migrations before increasing the format version.

The intended stable manifest contract is:

- `format`: `MIR4D`
- `formatVersion`: integer format version
- `appVersion`: application version
- `name`: project name
- `createdAt`: ISO-8601 creation date
- `modifiedAt`: ISO-8601 modification date
- `uuid`: stable project identifier
- `modelPath`: relative path to the model document
- UI/session fields such as workbench, sub-mode, tree selection, grid, axes, section mode and current time

When these additional identity/path fields are introduced into the Swift manifest, opening code must provide migration/default handling for existing version-1 packages rather than making old projects unreadable.

## Model

`Scenes/model.mir4d.json` contains the parameterized MIR 4D model. The model schema has its own schema version and is independent from the project manifest.

Recommended future separation:

- JSON: topology-independent parameters, model tree, bodies and operations
- `Models/`: imported mesh/CAD assets and future binary B-Rep/tessellation data
- `Results/`: simulation/calculation output

Large assemblies may later be sharded by body/operation UUID without changing the external `.mir4d` identity.

## Saving rules

- Explicit `Cmd-S` performs a full project save.
- Autosave uses the model-only scope when only model data is dirty.
- JSON is compact (`sortedKeys`, ISO-8601 dates) for production saves.
- Atomic replacement is mandatory for project JSON files.
- Encoding and model file I/O may run off the main actor.
- Model revision checks prevent writing an unchanged model.

## Compatibility

`formatVersion` is the project-format version. `schemaVersion` is the model-schema version. They are independent.

Rules:

1. Never silently open a newer unsupported format.
2. Never remove support for an older format without a migration.
3. Validate the manifest before loading model data.
4. Keep package-internal paths relative to the package root.
5. Do not store absolute machine-specific asset paths in the manifest.

## macOS integration

The intended public UTI is:

`app.mir4d.project`

It should conform to `com.apple.package` and advertise the `.mir4d` filename extension. The application should register the UTI and document type in the macOS application bundle so Finder, Open panels and Save panels identify MIR 4D projects as native packages.

## Future archive

A single-file `.mir4d` archive can be implemented later as an exchange format containing the same package tree. The native editing format remains the package because autosave can update individual files without repacking the entire project.
