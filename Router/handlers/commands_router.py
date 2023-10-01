from aiogram import Router, md
from aiogram.types import Message
from aiogram.filters.command import Command

import grpc
import FrontAPI_pb2_grpc
import FrontAPI_pb2

router = Router()

@router.message(Command("new"))
async def cmd_start(message: Message):
    async with grpc.aio.insecure_channel("localhost:50051") as channel:
        stub = FrontAPI_pb2_grpc.CalendarStub(channel)

        request = FrontAPI_pb2.StartNewRequest()

        request.user.id = message.from_user.id

        response = await stub.StartNew(request)
    await message.answer(md.quote(response.status.ok.text))

@router.message(Command("remove"))
async def cmd_remove(message: Message):
    left = message.text.removeprefix('/remove').strip()
    try:
        value = int(left)
        if  str(value) != left:
            raise ValueError("Not an int64")
    except ValueError:
        await message.answer("Переданный аргумент не является числом")
        return

    async with grpc.aio.insecure_channel("localhost:50051") as channel:
        stub = FrontAPI_pb2_grpc.CalendarStub(channel)

        request = FrontAPI_pb2.StartRemoveRequest()

        request.user.id = message.from_user.id
        request.id      = value

        response = await stub.StartRemove(request)
    await message.answer(md.quote(response.status.ok.text))

@router.message(Command("showall"))
async def cmd_showall(message: Message):
    async with grpc.aio.insecure_channel("localhost:50051") as channel:
        stub = FrontAPI_pb2_grpc.CalendarStub(channel)

        request = FrontAPI_pb2.ShowAllRequest()

        request.user.id = message.from_user.id

        response = await stub.ShowAll(request)
    await message.answer(md.quote(response.status.ok.text))

if __name__ == "__main__":
    import os

    os.abort()
