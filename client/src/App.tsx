import React from 'react';

import { DebtSimplifierClient } from 'proto/service_grpc_web_pb';
import { TestReq } from 'proto/service_pb';

export function App() {
  const client = new DebtSimplifierClient('cknittel.com:3001', null, null);
  const req = new TestReq();
  req.setMsg('test guy');
  client.test(req, undefined, (_error, res) => {
    console.log(`Got response : ${res.getMsg()}`);
  });
  return <></>;
}
