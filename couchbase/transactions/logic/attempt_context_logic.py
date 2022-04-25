from typing import TYPE_CHECKING, Optional

from couchbase.pycbc_core import (transaction_op,
                                  transaction_operations,
                                  transaction_query_op)

if TYPE_CHECKING:
    from asyncio import AbstractEventLoop

    from couchbase._utils import PyCapsuleType
    from couchbase.serializer import Serializer


class AttemptContextLogic:
    def __init__(self,
                 ctx,    # type: PyCapsuleType
                 loop,    # type: Optional[AbstractEventLoop]
                 serializer  # type: Serializer
                 ):
        print(f'creating new attempt context with context {ctx} and loop {loop}, and serializer {serializer}')
        self._ctx = ctx
        self._loop = loop
        self._serializer = serializer

    def get(self, coll, key, **kwargs):
        kwargs.update(coll._get_connection_args())
        kwargs.pop("conn")
        kwargs["key"] = key
        kwargs["ctx"] = self._ctx
        kwargs["op"] = transaction_operations.GET.value
        print(f'get calling transaction op with {kwargs}')
        return transaction_op(**kwargs)

    def insert(self, coll, key, value, **kwargs):
        kwargs.update(coll._get_connection_args())
        kwargs.pop("conn")
        kwargs["key"] = key
        kwargs["ctx"] = self._ctx
        kwargs["op"] = transaction_operations.INSERT.value
        kwargs["value"] = self._serializer.serialize(value)
        print(f'insert calling transaction op with {kwargs}')
        return transaction_op(**kwargs)

    def replace(self, txn_get_result, value, **kwargs):
        kwargs.update({"ctx": self._ctx, "op": transaction_operations.REPLACE.value,
                       "value": self._serializer.serialize(value),
                       "txn_get_result": txn_get_result._res})
        print(f'replace calling transaction op with {kwargs}')
        return transaction_op(**kwargs)

    def remove(self, txn_get_result, **kwargs):
        kwargs.update({"ctx": self._ctx,
                       "op": transaction_operations.REMOVE.value,
                       "txn_get_result": txn_get_result._res})
        print(f'remove calling transaction op with {kwargs}')
        return transaction_op(**kwargs)

    def query(self, query, options, **kwargs):
        kwargs.update({"ctx": self._ctx, "statement": query, "options": options._base})
        print(f'query calling transaction_op with {kwargs}')
        return transaction_query_op(**kwargs)
