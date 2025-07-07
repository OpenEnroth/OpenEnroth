#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Interface/ReadOnlyFileSystem.h"

/**
 * Merges several filesystems into a single read-only view:
 * - Read operations go one by one over the underlying filesystems. This effectively means that files on filesystem #0
 *   will hide the files on filesystem #1 that have the same names.
 * - Folder contents are merged.
 * - The resulting view is kind of a Schrodinger's filesystem because it can contain files and folders with the same
 *   name.
 * - In case there is a file and a folder with the same name, `stat()` returns information for a file, so that it's
 *   possible to know its size. `ls()` will return both entries, in unspecified order.
 *
 * Some notes on why go Schrodingermaxxxing. When there is a conflict between underlying filesystems, and we have
 * a file and a folder with the same names, we have several different options:
 * 1. Only the file is visible.
 * 2. Only the folder is visible.
 * 3. Whichever is first in the filesystem list, is visible.
 * 4. Conflicting path is not visible through the merged filesystem at all.
 * 5. Schrodingermaxxx and make both visible.
 * 
 * When picking between the options, we must remember that:
 * a. A tree of `MergingFileSystem`s should behave the same way as a single flattened `MergingFileSystem`. E.g. if we
 *    have a `MergingFileSystem` containing another `MergingFileSystem`, it should be no different from just having
 *    a single `MergingFileSystem` with all the leaf filesystems added to it.
 * b. Ideally, each method of the merging filesystem should call into each of the base filesystems at most once.
 * c. The behavior shouldn't be surprising.
 * 
 * Point #1 above fails (b) and (c). We can fix (b) by introducing a `walk()` method to `FileSystem` that would
 * return the longest existing prefix of a path, but then we're still left with the fact that behaviour is surprising.
 * 
 * Point #2 fails (c).
 * 
 * Point #3 fails (a), (b) and (c). How exactly it does it fail (a)? Imagine the following sequence of conflicts:
 * folder `x`, file `x`, then again folder `x` and file `x`, with the first pair in the first `MergingFileSystem` and
 * the second pair in the second `MergingFileSystem`, which are then merged into a single common `MergingFileSystem`.
 * The common `MergingFileSystem` will merge the contents of the folders in this case, which (supposedly) would not
 * be the case if all the conflicts were to happen inside a single `MergingFileSystem`. This can be worked around, but
 * the workaround is questionable.
 * 
 * Point #4 fails (b), can be fixed in the same way as point #1. So, a viable option.
 * 
 * Point #5 satisfies all the criteria, even though it's suffering a low-key bipolar disorder. So this is what we do.
 */
class MergingFileSystem : public ReadOnlyFileSystem {
 public:
    explicit MergingFileSystem(std::vector<const FileSystem *> bases);
    virtual ~MergingFileSystem();

    // TODO(captainurist): think about smth like a displayPriority for _displayPath? Basically a FS that you want to
    //                     forward displayPath calls to if there are conflicts (no files exist / multiple files exist).

 private:
    virtual bool _exists(FileSystemPathView path) const override;
    virtual FileStat _stat(FileSystemPathView path) const override;
    virtual void _ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(FileSystemPathView path) const override;
    virtual std::unique_ptr<InputStream> _openForReading(FileSystemPathView path) const override;
    virtual std::string _displayPath(FileSystemPathView path) const override;

    const FileSystem *locateForReading(FileSystemPathView path) const;
    const FileSystem *locateForReadingOrNull(FileSystemPathView path) const;

 private:
    std::vector<const FileSystem *> _bases;
};
