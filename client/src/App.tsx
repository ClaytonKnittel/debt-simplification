import React from 'react';

import{DebtSimplifierClient} from 'proto/service_grpc_web_pb';
import from 'proto/service_pb';

export function App() {
  const client = new DebtSimplifierClient('cknittel.com:3001', null, null);
  const req = new proto.debt_simpl.TestReq();
  req.setMsg('test guy');
  client.test(
      req, null, (err, res) = > {
        console.log(`Got response : $ { res.msg() }`);
      });
  return <></>;
}
