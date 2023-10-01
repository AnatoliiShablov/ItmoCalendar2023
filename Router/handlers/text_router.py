from aiogram import Router, md
from aiogram.types import Message

import grpc
import FrontAPI_pb2_grpc
import FrontAPI_pb2

router = Router()


@router.message()
async def argument_text(message: Message):
    async with grpc.aio.insecure_channel("localhost:50051") as channel:
        stub = FrontAPI_pb2_grpc.CalendarStub(channel)

        request = FrontAPI_pb2.AddNextArgumentRequest()

        request.user.id = message.from_user.id
        request.text    = message.text.strip()

        response = await stub.AddNextArgument(request)
    await message.answer(md.quote(response.status.ok.text))

if __name__ == "__main__":
    import os

    os.abort()
