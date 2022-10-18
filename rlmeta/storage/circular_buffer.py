# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from typing import Callable, Optional, Sequence, Tuple, Union

import numpy as np

import rlmeta.utils.nested_utils as nested_utils
import _rlmeta_extension

from rlmeta.core.types import NestedTensor, Tensor
from rlmeta.storage import Storage


class CircularBuffer(Storage):

    def __init__(
        self,
        capacity: int,
        collate_fn: Optional[Callable[[Sequence[NestedTensor]],
                                      NestedTensor]] = None
    ) -> None:
        self._impl = _rlmeta_extension.CircularBuffer(capacity)
        self._collate_fn = collate_fn

    def __getitem__(
            self,
            key: Union[int,
                       Tensor]) -> Union[NestedTensor, Sequence[NestedTensor]]:
        ret = self._impl[key]
        if not isinstance(key, int) and self._collate_fn is not None:
            ret = nested_utils.collate_nested(self._collate_fn, ret)
        return ret

    @property
    def capacity(self) -> int:
        return self._impl.capacity

    @property
    def size(self) -> int:
        return self._impl.size

    def reset(self) -> None:
        self._impl.reset()

    def clear(self) -> None:
        self._impl.clear()

    def append(self, data: NestedTensor) -> Tuple[int, Optional[int]]:
        return self._impl.append(data)

    def extend(self,
               data: Sequence[NestedTensor]) -> Tuple[np.ndarray, np.ndarray]:
        return self._impl.extend(data)
