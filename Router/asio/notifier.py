from aiogram import Bot, md

import grpc
import FrontAPI_pb2_grpc
import FrontAPI_pb2


async def notifier(my_bot: Bot):
    async with grpc.aio.insecure_channel("localhost:50052") as channel:
        async for ans in FrontAPI_pb2_grpc.NotifierStub(channel).Subscribe(FrontAPI_pb2.SubscribeRequest()):
            await my_bot.send_message(ans.user.id, md.quote(ans.text))
