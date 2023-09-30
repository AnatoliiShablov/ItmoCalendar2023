from aiogram import Router, md
from aiogram.types import Message

import grpc
import FrontAPI_pb2_grpc
import FrontAPI_pb2

router = Router()


@router.message()
async def simple_text(message: Message):
    async with grpc.aio.insecure_channel("localhost:50051") as channel:
        stub = FrontAPI_pb2_grpc.CalendarStub(channel)
        request = FrontAPI_pb2.User()
        request.id = message.from_user.id
        response = await stub.StartNew(request)
    await message.answer(md.quote(response.text))

    #    @dp.message(Command("start"))
    #    async def cmd_start(message: types.Message):
    #        await message.answer(f"Hello: *{message.from_user.id}*\\!")

    #    @dp.message(Command("dice"))
    #    async def cmd_dice(message: types.Message, command: CommandObject):
    #        await message.answer_dice()
    #        if command.args is not None:
    #            await message.answer(f"{md.quote(command.args)} {message.md_text.split(maxsplit=1)[1]}")


if __name__ == "__main__":
    import os

    os.abort()
