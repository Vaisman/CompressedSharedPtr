# CompressedSharedPtr

Compressed shared_ptr

handle is a 64-bit integer where:
 - Lower 42 bits represent the pointer bits.
 - Upper 22 bits represent the reference count.

```cpp
//  +----------------------+----------------------+
//  |   22 bits refcount  |   42 bits pointer     |
//  +----------------------+----------------------+
//
//   bit layout example:
//   [ 63 ....................  42 | 41 ................................ 0 ]
//   [         refCount            |              rawPtr                   ]
