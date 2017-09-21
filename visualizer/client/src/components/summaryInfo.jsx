import React from 'react';
import PropTypes from 'prop-types';
import Dialog, { DialogTitle, DialogContent } from 'material-ui/Dialog';
import List, { ListItem, ListItemText } from 'material-ui/List';
import Divider from 'material-ui/Divider';


function SummaryInfo(props) {
  const { ...other } = props;
  return (
    <Dialog onRequestClose={props.onRequestClose} {...other}>
      <DialogTitle>Instructions Trace</DialogTitle>
      <DialogContent>
        <List dense>
          {props.content.map((v, idx) =>
            (
              <div key={idx}>
                <Divider />
                <ListItem>
                  <ListItemText
                    primary={v.instr}
                    secondary={`Line No. ${v.instr_loc}`}
                  />
                </ListItem>
                <Divider />
              </div>
            ),
          )}
        </List>
      </DialogContent>
    </Dialog>
  );
}

SummaryInfo.propTypes = {
  content: PropTypes.arrayOf(PropTypes.object).isRequired,
  onRequestClose: PropTypes.func.isRequired,
};

export default SummaryInfo;
